# 안전영역

## 시험장 환경 모델
- 표준 모델 International Standard Atmosphere (ISA) as defined by ISO 2533
	- 압력: 𝑃(ℎ)
	- 온도: T(h)
	- 밀도: ρ(h)
	- 바람: u(h) (east-west), v(h) (north-south)
- 유도무기 시험 (0~20,000m)
	- 대류권(0~11,000m, 온도에 따라 일정한 변화), 권계층(11,000~20,000m, 온도 일정)
- 풍향, 풍속을 상수로 적용 -> 현실적인 방법 적용
	- 기상예보, 과거기상 데이터 활용

## RocketPy 환경 모델
- GFS/ERA5, 앙상블(Ensemble)
	- ERA5는 유럽 중기예보센터(European Centre for Medium-Range Weather Forecasts, ECMWF)가 생산한 최신 전 지구 재분석(reanalysis) 기후 데이터세트다. Climate Data Store(CDS)를 통해 무료로 제공되며, 매일 업데이트되어 약 5일 전까지의 ‘near-real-time’ 데이터를 포함한다.
- 룩업테이블 생성
    - “Geopotential”, “Temperature”, “U-wind”, “V-wind”
	- NetCDF (Network Common Data Form)
- 유도 무기 비행 시간 동안 전시 시간 또는 시뮬레이션 시간 시 참조(고도에 따라 보간법)

## 목표
- Atmosphere = f(latitude,longitude,altitude,time) 룩업 테이블 생성
- 항력(Drag) 계산
	- $$ D = \frac{1}{2}\rho V^2 C_d A $$
	- $$ \vec{V}_{rel}=\vec{V}_{body} − \vec{V}_{wind} $$

## 구현


## 향후 연구
- 실시간 환경 데이터 참조
- API 지원, DDS 서비스
