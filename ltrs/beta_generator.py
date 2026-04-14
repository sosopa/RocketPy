import numpy as np

def generate_beta_set(length, diameter, total_mass, n_classes=6):
    """
    Generate realistic ballistic coefficient (beta) set.

    Parameters:
        length (m): missile length
        diameter (m): missile diameter
        total_mass (kg): total mass of missile
        n_classes (int): number of debris classes

    Returns:
        beta_set (np.array): beta values (kg/m^2)
    """

    # ---- Assumptions ----
    Cd_mean = 1.4  # average drag coefficient (tumbling debris)

    # Reference cross-sectional area (max frontal area)
    A_ref = np.pi * (diameter / 2)**2

    # ---- Step 1: Mass distribution (log scale) ----
    # From very small fragments to large chunks
    m_min = total_mass * 0.0001   # 0.01%
    m_max = total_mass * 0.3      # 30%

    masses = np.logspace(np.log10(m_min), np.log10(m_max), n_classes)

    # ---- Step 2: Area scaling ----
    # Small fragments: larger area-to-mass ratio
    # Large fragments: closer to body cross-section
    area_scale = np.linspace(3.0, 0.5, n_classes)  # heuristic

    areas = A_ref * area_scale

    # ---- Step 3: Compute beta ----
    beta = masses / (Cd_mean * areas)

    # ---- Step 4: Optional Improvements ----
    # Add randomness
    beta *= np.exp(np.random.normal(0, 0.3, size=n_classes))

    return beta, masses, areas

beta, masses, areas = generate_beta_set(
    length=5.0,      # meters
    diameter=0.3,    # meters
    total_mass=300,  # kg
    n_classes=6
)

print("beta:", beta)
print("masses:", masses)
print("areas:", areas)
