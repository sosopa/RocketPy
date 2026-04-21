#include <Eigen/Dense>
using namespace Eigen;

MatrixXd computeCovariance(const std::vector<XY>& pts, Vector2d& mean) {

    int N = pts.size();

    mean.setZero();

    for (const auto& p : pts) {
        mean(0) += p.x;
        mean(1) += p.y;
    }
    mean /= N;

    MatrixXd cov = MatrixXd::Zero(2,2);

    for (const auto& p : pts) {
        Vector2d d;
        d << p.x - mean(0), p.y - mean(1);
        cov += d * d.transpose();
    }

    cov /= N;  // or (N-1) if you want unbiased

    return cov;
}

void computeEllipse(
    const Matrix2d& cov,
    const Vector2d& mean,
    double& major_axis,
    double& minor_axis,
    double& angle)
{
    SelfAdjointEigenSolver<Matrix2d> solver(cov);

    Vector2d eigenvalues = solver.eigenvalues();
    Matrix2d eigenvectors = solver.eigenvectors();

    // χ² value for 95% confidence (2 DOF)
    double chi2 = 5.991;

    // Axes lengths
    major_axis = std::sqrt(chi2 * eigenvalues(1)); // larger eigenvalue
    minor_axis = std::sqrt(chi2 * eigenvalues(0));

    // Orientation (angle from x-axis)
    Vector2d major_vec = eigenvectors.col(1);
    angle = std::atan2(major_vec(1), major_vec(0));
}

std::vector<XY> generateEllipsePoints(
    const Vector2d& mean,
    double a, double b, double angle,
    int n = 100)
{
    std::vector<XY> pts;

    for (int i = 0; i < n; ++i) {
        double t = 2.0 * M_PI * i / n;

        double x = a * cos(t);
        double y = b * sin(t);

        // rotate
        double xr = cos(angle)*x - sin(angle)*y;
        double yr = sin(angle)*x + cos(angle)*y;

        pts.push_back({ mean(0) + xr, mean(1) + yr });
    }

    return pts;
}

ImpactPoint xyToLatLon(const XY& p, const ImpactPoint& ref) {
    ImpactPoint out;

    out.lat = ref.lat + p.y / EARTH_RADIUS;
    out.lon = ref.lon + p.x / (EARTH_RADIUS * cos(ref.lat));

    return out;
}
