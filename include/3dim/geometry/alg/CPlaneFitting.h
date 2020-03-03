#ifndef CPlaneFitting_H_included
#define CPlaneFitting_H_included

#if defined (TRIDIM_USE_CERES)

#include <geometry/base/types.h>
#include <geometry/base/CPlane.h>

#include <ceres/ceres.h>


namespace geometry
{
    class CPlaneFitting
    {
    public:
        //! Simple constructor
        CPlaneFitting();

        bool apply(const Vec3Array &points, CPlane &plane);

    private:
        typedef std::array<double, 4> tDoubleQuad;

        
        struct CCostFunctor
        {
            typedef ceres::AutoDiffCostFunction<CCostFunctor, 1, 4> tCostFunction;

            // Point position
            double x, y, z;

            CCostFunctor() : x(0.0), y(0.0), z(0.0) {}
            CCostFunctor(const CCostFunctor &f) : x(f.x), y(f.y), z(f.z) {}
            CCostFunctor(const Vec3 &point)
                : x(point[0]), y(point[1]), z(point[2]) {}

            #define SQ(t) (t)*(t)
            
            template<typename T>
            bool operator() (const T* parameters, T *residual) const
            {
                
                T divider(sqrt(SQ(parameters[0]) + SQ(parameters[1]) + SQ(parameters[2]) + SQ(parameters[3])));

                if(divider > T(std::numeric_limits<double>::min()))
                {
                    residual[0] = abs(T(x) * parameters[0] + T(y) * parameters[1] + T(z) * parameters[2] + parameters[3]) / divider;
                    return true;
                }

                return false;
            }

            static ceres::CostFunction * create(const Vec3 &point)
            {
                return (new tCostFunction(
                    new CCostFunctor(point)));
            }

        protected:
            template<typename T, int N = 1>
            double toDouble(T d) const { return d; }

            template<int N = 1>
            double toDouble(ceres::Jet<double, N> d) const { return d.a; }

            

        }; // struct CCostFunctor

        //! Vector of ceres cost functions
        
        typedef std::vector<ceres::CostFunction *> tCostFunctionPtrVec;

    private:
        //! Is it even possible to fit plane to the input array of points?
        bool isPossibleToFit(const Vec3Array &points);

        //! Try to fit plane to the array
        bool fit(const Vec3Array &points, CPlane &plane);

        void planeToArray(const CPlane &input, tDoubleQuad &output);
        void arrayToPlane(const tDoubleQuad &input, CPlane &output);
    };
}

// CPlaneFitting_H_included
#endif

#endif
