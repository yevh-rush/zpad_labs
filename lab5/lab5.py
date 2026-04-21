import streamlit as st
import pandas as pd
import plotly.express as px
import os
import re

st.set_page_config(page_title="NOAA Data Analysis", layout="wide")
st.title("Аналіз вегетаційних індексів (VCI, TCI, VHI)")

@st.cache_data
def load_data(folder_path="data"):
    all_files = [f for f in os.listdir(folder_path) if f.endswith('.csv')]
    df_list = []
    
    province_dict = {
        1: 'Черкаська', 2: 'Чернігівська', 3: 'Чернівецька', 4: 'Республіка Крим', 
        5: 'Дніпропетровська', 6: 'Донецька', 7: 'Івано-Франківська', 8: 'Харківська', 
        9: 'Херсонська', 10: 'Хмельницька', 11: 'Київська', 12: 'м. Київ', 
        13: 'Кіровоградська', 14: 'Луганська', 15: 'Львівська', 16: 'Миколаївська', 
        17: 'Одеська', 18: 'Полтавська', 19: 'Рівненська', 20: 'м. Севастополь', 
        21: 'Сумська', 22: 'Тернопільська', 23: 'Закарпатська', 24: 'Вінницька', 
        25: 'Волинська', 26: 'Запорізька', 27: 'Житомирська'
    }
    
    for file in all_files:
        file_path = os.path.join(folder_path, file)
        
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        prov_id = 0
        parsed_data = []
        
        for line in lines:
            if 'Province=' in line:
                match = re.search(r'Province=\s*(\d+)', line)
                if match:
                    prov_id = int(match.group(1))
            
            clean_line = re.sub(r'<[^>]+>', '', line).strip()
            
            parts = [p.strip() for p in clean_line.split(',') if p.strip()]
            
            if len(parts) >= 7 and parts[0].isdigit():
                parsed_data.append(parts[:7])
        
        if parsed_data:
            df = pd.DataFrame(parsed_data, columns=['YEAR', 'WEEK', 'SMN', 'SMT', 'VCI', 'TCI', 'VHI'])
            
            for col in df.columns:
                df[col] = pd.to_numeric(df[col], errors='coerce')
                
            df['PROVINCE_ID'] = prov_id
            df['PROVINCE_NAME'] = df['PROVINCE_ID'].map(province_dict).fillna(f'Область {prov_id}')
            
            df_list.append(df)
            
    if not df_list:
        return pd.DataFrame()
        
    full_df = pd.concat(df_list, ignore_index=True)
    
    full_df = full_df.dropna(subset=['VCI', 'TCI', 'VHI', 'YEAR', 'WEEK'])
    full_df = full_df[full_df['VHI'] > 0]
    
    return full_df

df = load_data()

if df.empty:
    st.error("Помилка: Дані не завантажились! Перевірте файли у папці data.")
    st.stop()

def reset_filters():
    for key in ['idx', 'prov', 'yr_range', 'wk_range', 's_asc', 's_desc']:
        if key in st.session_state:
            del st.session_state[key]

col_controls, col_results = st.columns([1, 3])

with col_controls:
    st.header("Налаштування")
    
    index_options = ['VCI', 'TCI', 'VHI']
    selected_index = st.selectbox("Виберіть індекс", index_options, key='idx')
    
    provinces = sorted(df['PROVINCE_NAME'].unique())
    selected_province = st.selectbox("Виберіть область", provinces, key='prov')
    
    min_year, max_year = int(df['YEAR'].min()), int(df['YEAR'].max())
    year_range = st.slider("Інтервал років", min_year, max_year, (min_year, max_year), key='yr_range')
    
    week_range = st.slider("Інтервал тижнів", 1, 52, (1, 52), key='wk_range')
    
    st.write("Сортування даних:")
    sort_asc = st.checkbox("За зростанням", key='s_asc')
    sort_desc = st.checkbox("За спаданням", key='s_desc')
    
    st.button("Скинути всі фільтри", on_click=reset_filters, type="primary")

filtered_df = df[
    (df['PROVINCE_NAME'] == selected_province) &
    (df['YEAR'] >= year_range[0]) & (df['YEAR'] <= year_range[1]) &
    (df['WEEK'] >= week_range[0]) & (df['WEEK'] <= week_range[1])
].copy()

if sort_asc and sort_desc:
    st.warning("Увімкнено обидва чекбокси сортування. Сортування скасовано.")
elif sort_asc:
    filtered_df = filtered_df.sort_values(by=selected_index, ascending=True)
elif sort_desc:
    filtered_df = filtered_df.sort_values(by=selected_index, ascending=False)

with col_results:
    tab1, tab2, tab3 = st.tabs(["Таблиця даних", "Динаміка індексу", "Порівняння областей"])
    
    with tab1:
        st.subheader(f"Дані {selected_index} для {selected_province}")
        st.dataframe(filtered_df[['YEAR', 'WEEK', 'PROVINCE_NAME', 'VCI', 'TCI', 'VHI']], use_container_width=True, hide_index=True)
        
    with tab2:
        st.subheader(f"Часовий ряд {selected_index} ({year_range[0]}-{year_range[1]})")
        plot_df = filtered_df.sort_values(by=['YEAR', 'WEEK']).copy()
        plot_df['Час'] = plot_df.apply(lambda row: f"{int(row['YEAR'])}-W{int(row['WEEK']):02d}", axis=1)
        
        fig_line = px.line(
            plot_df, x='Час', y=selected_index, markers=True,
            title=f"Динаміка {selected_index} для {selected_province}",
            labels={'Час': 'Рік і Тиждень', selected_index: 'Значення індексу'}
        )
        st.plotly_chart(fig_line, use_container_width=True)
        
    with tab3:
        st.subheader(f"Порівняння {selected_index} з іншими областями")
        comp_df = df[
            (df['YEAR'] >= year_range[0]) & (df['YEAR'] <= year_range[1]) &
            (df['WEEK'] >= week_range[0]) & (df['WEEK'] <= week_range[1])
        ]
        agg_df = comp_df.groupby('PROVINCE_NAME')[selected_index].mean().reset_index()
        agg_df['Колір'] = agg_df['PROVINCE_NAME'].apply(lambda x: 'Обрана область' if x == selected_province else 'Інші області')
        
        fig_bar = px.bar(
            agg_df, x='PROVINCE_NAME', y=selected_index, color='Колір',
            color_discrete_map={'Обрана область': '#EF553B', 'Інші області': '#636EFA'},
            title=f"Середнє значення {selected_index} за обраний період",
            labels={'PROVINCE_NAME': 'Область', selected_index: f'Середнє {selected_index}'}
        )
        fig_bar.update_layout(xaxis={'categoryorder':'total descending'})
        st.plotly_chart(fig_bar, use_container_width=True)
