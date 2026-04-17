# 안전영역
- 기상 데이터를 활용한 해상 유도무기 시험 안전영역 설정 기법 연구
- A Study on Safety Zone Determination for Maritime Guided Weapon Tests Using Atmospheric Data

## 초록
- 유도무기시험에서 기상 환경 데이터를 활용하는 것은 시험의 안정성을 높이고 사전 분석과 사후 분석의 정확성을 향상시키는데 매우 중요한 부분이다. 하지만 과거 유도무기 자유도 시뮬레이션 모델 개발에서 풍향, 풍속과 같은 상수값을 사용하여 단순화하거나 표준 대기 모델과 같은 정형화된 모델을 사용하는 경우가 많았다. 본 연구에서는 기상예보와 과거 기상 데이터를 사용하여 보다 현실적인 방법을 적용하는 관련 연구를 살펴 보았고, 해상유도무기시험에서 안전영역 설정을 위해 적용한 방법에 대하여 기술하였다.
- The utilization of atmospheric environmental data in guided weapon testing plays a critical role in enhancing test safety and improving the accuracy of both pre-test and post-test analyses. However, in the past, the development of guided weapon flight dynamics simulation models has often relied on simplified approaches, such as the use of constant values for wind direction and wind speed, or standardized atmospheric models. In this study, previous research incorporating weather forecasting and historical atmospheric data to achieve more realistic modeling approaches is reviewed. Furthermore, a practical method applied to the establishment of safety zones in maritime guided weapon testing is presented, demonstrating its effectiveness in reflecting realistic environmental conditions.

## 해상시험장 대기 모델
- 표준 모델 International Standard Atmosphere (ISA) as defined by ISO 2533
	- 압력 P(h), 온도 T(h), 밀도 ρ(h), 바람(east-west) u(h), 바람(north-south) v(h)
- 유도무기 시험 (0~20,000m)
	- 대류권(Troposphere, ~11km, 온도에 따라 일정한 변화), 대류권 계층(Tropopause, ~20km, 온도 일정), 성층권
- 풍향, 풍속을 상수로 적용 -> 기상예보, 과거기상 데이터 활용한 현실적인 방법 적용

## 관련 연구
- RocketPy 대기 모델
	- 표준 대기
	- 기상 예보 GFS
	- 고도 프로파일링 Atmospheric Soundings 
	- 과거 재분석 ERA5 Reanalysis
	- Ensemble
- ERA5
	- ERA5는 유럽 중기예보센터(European Centre for Medium-Range Weather Forecasts, ECMWF)가 생산한 최신 전 지구 재분석(reanalysis) 기후 데이터세트다. Climate Data Store(CDS)를 통해 무료로 제공되며, 매일 업데이트되어 약 5일 전까지의 ‘near-real-time’ 데이터를 포함한다.
- 룩업테이블 생성
    - Geopotential, Temperature, U-wind, V-wind
	- NetCDF (Network Common Data Form) 형식
- 유도 무기 비행 시간 동안 고도에 따라 룩업테이블을 보간법 사용하여 참조
	- 전시 화면 갱신 주기, 시뮬레이션 타임 스텝

## 구현
- Atmosphere = f(latitude, longitude, altitude, time) 데이터셋 확보
	- 2026년3월25일20시 ERA5 Reanalysis
- 고도에 대한 룩업 테이블 생성
- 항력(Drag) 계산에 적용
	- $$ D = \frac{1}{2}\rho V^2 C_d A $$
	- $$ \vec{V}_{rel}=\vec{V}_{body} − \vec{V}_{wind} $$
	- $$ \vec{D} = -\frac{1}{2}\rho V^2 C_d A \cdot \hat{V}_{rel}$$
- ISA 방식 보완

## 향후 연구
- 앙상블(Ensemble) 데이터를 활용한 몬테칼로 시뮬레이션
	- Ensemble member(10개), mean, spread
- 실시간에 가까운 환경 데이터 참조 시스템 구축
	- DDS 서비스, API 지원

## 관련 연구
- 이 논문은 2026년 정부의 재원으로 수행된 연구 결과임
- 박현성, 송상헌, 김영주, "해상유도무기시험 안전영역설정 및 실시간 파편영역예측 설계 및 구현", 한국군사과학기술학회 추계학술대회, 2022.
- Giovani H. Ceotto, et al., RocketPy: Six Degree-of-Freedom Rocket Trajectory Simulator, Journal of Aerospace Engineering, Vol 34, Issue 6, Aug 20, 2021.
- Copernicus Climate Change Service (C3S) Climate Data Store (CDS). DOI: 10.24381/cds.bd0915c6 (Accessed on 25-03-2026).
- David E. Outka, Robert A. LaFarge, "Mission hazard assessment for STARS Mission 1 (M1) in the Marshall Islands area", SAND93-0218, 1993.

## 데이터셋
- BR 
- HR
- SM
