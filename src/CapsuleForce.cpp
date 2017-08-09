
#include "CapsuleForce.hpp"
#include "TypeSixSecretionEnumerations.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/point.hpp>

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
CapsuleForce<ELEMENT_DIM,SPACE_DIM>::CapsuleForce()
    : AbstractForce<ELEMENT_DIM,SPACE_DIM>()
{
    assert(ELEMENT_DIM == 2u);
    assert(SPACE_DIM == 2u);
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
double CapsuleForce<ELEMENT_DIM,SPACE_DIM>::CalculateDistanceBetweenCapsules(
        Node<SPACE_DIM>& rNodeA,
        Node<SPACE_DIM>& rNodeB)
{
    using geom_point = boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>;
    using geom_segment = boost::geometry::model::segment<geom_point>;

    auto location_a = rNodeA.rGetLocation();
    auto location_b = rNodeB.rGetLocation();

    const double angle_a = rNodeA.rGetNodeAttributes()[NA_ANGLE];
    const double length_a = rNodeA.rGetNodeAttributes()[NA_LENGTH];

    const double angle_b = rNodeB.rGetNodeAttributes()[NA_ANGLE];
    const double length_b = rNodeB.rGetNodeAttributes()[NA_LENGTH];

    geom_point capsule_a_end_1(location_a[0] + 0.5 * length_a * cos(angle_a),
                               location_a[1] + 0.5 * length_a * sin(angle_a));

    geom_point capsule_a_end_2(location_a[0] - 0.5 * length_a * cos(angle_a),
                               location_a[1] - 0.5 * length_a * sin(angle_a));

    geom_point capsule_b_end_1(location_b[0] + 0.5 * length_b * cos(angle_b),
                               location_b[1] + 0.5 * length_b * sin(angle_b));

    geom_point capsule_b_end_2(location_b[0] - 0.5 * length_b * cos(angle_b),
                               location_b[1] - 0.5 * length_b * sin(angle_b));

    geom_segment capsule_axis_a(capsule_a_end_1, capsule_a_end_2);
    geom_segment capsule_axis_b(capsule_b_end_1, capsule_b_end_2);

    return boost::geometry::distance(capsule_axis_a, capsule_axis_b);
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
double CapsuleForce<ELEMENT_DIM,SPACE_DIM>::CalculateOverlapBetweenCapsules(
        Node<SPACE_DIM>& rNodeA,
        Node<SPACE_DIM>& rNodeB,
        const double shortestDistance)
{
    const double radius_a = rNodeA.rGetNodeAttributes()[NA_RADIUS];
    const double radius_b = rNodeB.rGetNodeAttributes()[NA_RADIUS];

    return radius_a + radius_b - shortestDistance;
}

template <unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void CapsuleForce<ELEMENT_DIM, SPACE_DIM>::CalculateForceDirectionAndContactPoints(Node<SPACE_DIM>& rNodeA,
                                                                                   Node<SPACE_DIM>& rNodeB,
                                                                                   const double shortestDistance,
                                                                                   c_vector<double, SPACE_DIM>& rVecAToB,
                                                                                   double& rContactDistA,
                                                                                   double& rContactDistB)
{
    EXCEPT_IF_NOT(SPACE_DIM == 2);

    using geom_point = boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>;
    using geom_segment = boost::geometry::model::segment<geom_point>;

    rContactDistA = DOUBLE_UNSET;
    rContactDistB = DOUBLE_UNSET;

    auto location_a = rNodeA.rGetLocation();
    auto location_b = rNodeB.rGetLocation();

    const double angle_a = rNodeA.rGetNodeAttributes()[NA_ANGLE];
    const double length_a = rNodeA.rGetNodeAttributes()[NA_LENGTH];

    const double angle_b = rNodeB.rGetNodeAttributes()[NA_ANGLE];
    const double length_b = rNodeB.rGetNodeAttributes()[NA_LENGTH];

    geom_point capsule_a_end_1(location_a[0] + 0.5 * length_a * cos(angle_a),
                               location_a[1] + 0.5 * length_a * sin(angle_a));

    geom_point capsule_a_end_2(location_a[0] - 0.5 * length_a * cos(angle_a),
                               location_a[1] - 0.5 * length_a * sin(angle_a));

    geom_point capsule_b_end_1(location_b[0] + 0.5 * length_b * cos(angle_b),
                               location_b[1] + 0.5 * length_b * sin(angle_b));

    geom_point capsule_b_end_2(location_b[0] - 0.5 * length_b * cos(angle_b),
                               location_b[1] - 0.5 * length_b * sin(angle_b));

    geom_segment capsule_axis_a(capsule_a_end_1, capsule_a_end_2);
    geom_segment capsule_axis_b(capsule_b_end_1, capsule_b_end_2);

    c_vector<double, SPACE_DIM> end_point;
    c_vector<double, SPACE_DIM> centre_capsule;
    double angle_for_calculation;
    bool point_in_capsule_a;

    if (fabs(boost::geometry::distance(capsule_a_end_1, capsule_axis_b) - shortestDistance) < 1e-12)
    {
        end_point[0] = capsule_a_end_1.get<0>();
        end_point[1] = capsule_a_end_1.get<1>();
        centre_capsule = location_b;
        rContactDistA = 0.5 * length_a;
        angle_for_calculation = angle_b;
        point_in_capsule_a = true;
    }
    else if(fabs(boost::geometry::distance(capsule_a_end_2, capsule_axis_b) - shortestDistance) < 1e-12)
    {
        end_point[0] = capsule_a_end_2.get<0>();
        end_point[1] = capsule_a_end_2.get<1>();
        centre_capsule = location_b;
        rContactDistA = -0.5 * length_a;
        angle_for_calculation = angle_b;
        point_in_capsule_a = true;
    }
    else if(fabs(boost::geometry::distance(capsule_b_end_1, capsule_axis_a) - shortestDistance) < 1e-12)
    {
        end_point[0] = capsule_b_end_1.get<0>();
        end_point[1] = capsule_b_end_1.get<1>();
        centre_capsule = location_a;
        rContactDistB = 0.5 * length_b;
        angle_for_calculation = angle_a;
        point_in_capsule_a = false;
    }
    else if(fabs(boost::geometry::distance(capsule_b_end_2, capsule_axis_a) - shortestDistance) < 1e-12)
    {
        end_point[0] = capsule_b_end_2.get<0>();
        end_point[1] = capsule_b_end_2.get<1>();
        centre_capsule = location_a;
        rContactDistB = -0.5 * length_b;
        angle_for_calculation = angle_a;
        point_in_capsule_a = false;
    }
    else
    {
        NEVER_REACHED;
    }

    double cos_theta = cos(angle_for_calculation);
    double sin_theta = sin(angle_for_calculation);

    auto delta = end_point - centre_capsule;
    double other_length = (delta[0] * cos_theta + delta[1] * sin_theta) / (cos_theta * cos_theta + sin_theta * sin_theta);

    if (rContactDistA == DOUBLE_UNSET)
    {
        if (fabs(other_length) > 0.5 * length_a)
        {
            NEVER_REACHED;
            /*
             * We should never have to cap "other_length" in Capsule A: if we did, the implication would be that one of
             * the endpoints of Capsule A is involved in the closest distance, and "other_length" would then have to be
             * in Capsule B instead.
             */
            //other_length = copysign(0.5 * length_a, other_length);
        }

        rContactDistA = other_length;
    }
    else
    {
        if (fabs(other_length) > 0.5 * length_b)
        {
            other_length = copysign(0.5 * length_b, other_length);
        }

        rContactDistB = other_length;
    }

    c_vector<double, SPACE_DIM> cos_sin_theta;
    cos_sin_theta[0] = cos_theta;
    cos_sin_theta[1] = sin_theta;

    rVecAToB = delta - other_length * cos_sin_theta;

    if (point_in_capsule_a)
    {
        rVecAToB *= -1.0;
    }

    rVecAToB /= norm_2(rVecAToB);
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void CapsuleForce<ELEMENT_DIM,SPACE_DIM>::AddForceContribution(AbstractCellPopulation<ELEMENT_DIM,SPACE_DIM>& rCellPopulation)
{
    
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void CapsuleForce<ELEMENT_DIM,SPACE_DIM>::OutputForceParameters(out_stream& rParamsFile)
{
    // Call method on direct parent class
    AbstractForce<ELEMENT_DIM,SPACE_DIM>::OutputForceParameters(rParamsFile);
}

// Explicit instantiation
template class CapsuleForce<1,1>;
template class CapsuleForce<1,2>;
template class CapsuleForce<2,2>;
template class CapsuleForce<1,3>;
template class CapsuleForce<2,3>;
template class CapsuleForce<3,3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_ALL_DIMS(CapsuleForce)
