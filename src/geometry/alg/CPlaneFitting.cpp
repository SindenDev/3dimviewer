
#include <geometry/alg/CPlaneFitting.h>

#include <osg/dbout.h>
#include <array>

geometry::CPlaneFitting::CPlaneFitting()
{
    
}

bool geometry::CPlaneFitting::apply(const Vec3Array &points, CPlane &plane)
{
    if (!isPossibleToFit(points))
        return false;

    return fit(points, plane);
}

bool geometry::CPlaneFitting::isPossibleToFit(const Vec3Array &points)
{
    // Enough points?
    if (points.size() < 3)
        return false;

    return true;
}

bool geometry::CPlaneFitting::fit(const Vec3Array &points, CPlane &plane)
{
    // Prepare "starting plane"
    CPlane start_plane(Vec3(0.0, 1.0, 0.0), points.front());

    // Get start plane parameters
    tDoubleQuad parameters;

    planeToArray(start_plane, parameters);

    // Initialize solver
    ceres::Problem problem;

    // Prepare cost functions
//    prepareCostFunctions(problem, points, parameters);

    for (auto it = points.begin(); it != points.end(); ++it)
        problem.AddResidualBlock(CCostFunctor::create(*it), NULL, &parameters[0]);

    ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = true;
    ceres::Solver::Summary summary;

    ceres::Solve(options, &problem, &summary);
#ifdef _WIN32
    OutputDebugString(summary.BriefReport().c_str());
    OutputDebugString("\n");
#endif
    if (summary.final_cost < summary.initial_cost && summary.num_successful_steps > summary.num_unsuccessful_steps)
    {
        arrayToPlane(parameters, plane);
        return true;
    }

    return false;
}


void geometry::CPlaneFitting::planeToArray(const CPlane &input, tDoubleQuad &output)
{
    geometry::Vec4 parameters = input.getPlaneParameters();
    output[0] = parameters[0];
    output[1] = parameters[1];
    output[2] = parameters[2];
    output[3] = parameters[3];
}

void geometry::CPlaneFitting::arrayToPlane(const tDoubleQuad &input, CPlane &output)
{
    output.set(input[0], input[1], input[2], input[3]);
}

