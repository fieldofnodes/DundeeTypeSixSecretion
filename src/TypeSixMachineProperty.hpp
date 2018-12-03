
#ifndef TYPESIXMACHINEPROPERTY_HPP_
#define TYPESIXMACHINEPROPERTY_HPP_

#include <boost/shared_ptr.hpp>
#include "AbstractCellProperty.hpp"
#include "ChasteSerialization.hpp"
#include <boost/serialization/base_object.hpp>
#include "Exception.hpp"
#include "PetscTools.hpp"
#include <set>

/**
 * \todo Document class
 */
class TypeSixMachineProperty : public AbstractCellProperty
{
private:

    /** \todo Document member
	* CellTypeLabel is going to be an integer, for now it will be either 0 or 1. This refers to the TypeSixSecretion of attacker cells.
	* To goal is to have the capsules be different colors in ParaView as a result of the differreing labels.
	*/
	unsigned mCellTypeLabel;
	std::vector<std::pair<unsigned, std::vector<double>> > mMachineData;
    unsigned mNumMachineFiresInThisTimeStep;


public:

    /**
     * Constructor.
     */
    TypeSixMachineProperty();

    /**
     * Destructor.
     */
    virtual ~TypeSixMachineProperty();

    /**
     * @return #mMachineData
     */
    std::vector<std::pair<unsigned, std::vector<double>> > & rGetMachineData();
    void SetMachineData(std::vector<std::pair<unsigned, std::vector<double>> >);

    /**
     * @return #mGetCellTypeLabel
     */
    unsigned GetCellTypeLabel();
    void SetCellTypeLabel(unsigned);



    unsigned GetNumMachineFiresInThisTimeStep();
    void SetNumMachineFiresInThisTimeStep(unsigned numMachineFiresInThisTimeStep);

};

#include "SerializationExportWrapper.hpp"
// Declare identifier for the serializer
CHASTE_CLASS_EXPORT(TypeSixMachineProperty)

#endif /* TYPESIXMACHINEPROPERTY_HPP_ */
