/*

Copyright (c) 2005-2017, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "NodeBasedCellPopulationWithCapsules.hpp"
#include "ReplicatableVector.hpp"
#include "OdeLinearSystemSolver.hpp"
#include "UniformCellCycleModel.hpp"
#include "TypeSixSecretionEnumerations.hpp"

#include "Debug.hpp"
#include "AbstractCentreBasedDivisionRule.hpp"

#include "AbstractCentreBasedCellPopulation.hpp"
#include "RandomNumberGenerator.hpp"
#include "TypeSixMachineProperty.hpp"

template<unsigned DIM>
NodeBasedCellPopulationWithCapsules<DIM>::NodeBasedCellPopulationWithCapsules(NodesOnlyMesh<DIM>& rMesh,
                                      std::vector<CellPtr>& rCells,
                                      const std::vector<unsigned> locationIndices,
                                      bool deleteMesh)
    : NodeBasedCellPopulation<DIM>(rMesh, rCells, locationIndices, deleteMesh)
{

}

template<unsigned DIM>
NodeBasedCellPopulationWithCapsules<DIM>::NodeBasedCellPopulationWithCapsules(NodesOnlyMesh<DIM>& rMesh)
    : NodeBasedCellPopulation<DIM>(rMesh)
{
    // No Validate() because the cells are not associated with the cell population yet in archiving
}

template<unsigned DIM>
void NodeBasedCellPopulationWithCapsules<DIM>::UpdateNodeLocations(double dt)
{

	// Update cell lengths
		// Iterate over cell population

		for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->Begin();
		   cell_iter != this->End();
		   ++cell_iter)
		{


			  double cell_age  = cell_iter->GetAge();//+SimulationTime::Instance()->GetTimeStep();
			  double initial_length = 2.0;

			  if (bool(dynamic_cast<UniformCellCycleModel*>(cell_iter->GetCellCycleModel())))
			  {
				  double cell_cycle_time = static_cast<UniformCellCycleModel*>(cell_iter->GetCellCycleModel())->GetCellCycleDuration();

				  //if (cell_age >= cell_cycle_time)
				  //{
				//	  cell_age=0.0;
				//	  std::cout<< "Ahh " << "\n";
				 // }


				  //assert(bool(dynamic_cast<NodeBasedCellPopulation<DIM>*>(&mrCellPopulation)));
				  //Node<DIM>* pNodeA = dynamic_cast<NodeBasedCellPopulation<DIM>*>(&rCellPopulation)->GetNodeCorrespondingToCell(*cell_iter);

				  Node<DIM>* pNodeA = this->GetNodeCorrespondingToCell(*cell_iter);
				  double division_length = 2*initial_length + 2*pNodeA->rGetNodeAttributes()[NA_RADIUS];
				  double new_length = initial_length + (division_length - initial_length)*cell_age/cell_cycle_time;
				  //double new_length = initial_length*(1.0+cell_age/cell_cycle_time);


				  pNodeA->rGetNodeAttributes()[NA_LENGTH] = new_length;


			  }
		}


		NodeBasedCellPopulation<DIM>::UpdateNodeLocations(dt);
}

template<unsigned DIM>
CellPtr NodeBasedCellPopulationWithCapsules<DIM>::AddCell(CellPtr pNewCell, CellPtr pParentCell)
{

	auto pNewCellTemp=NodeBasedCellPopulation<DIM>::AddCell(pNewCell, pParentCell);



    // Create a new node
    Node<DIM>* p_new_node = this->GetNodeCorrespondingToCell(pNewCellTemp);// new Node<DIM>(this->GetNumNodes(), daughter_position, false); // never on boundary

    p_new_node->AddNodeAttribute(0.0);
    p_new_node->rGetNodeAttributes().resize(NA_VEC_LENGTH);

    double angle = (this->GetNodeCorrespondingToCell(pParentCell))->rGetNodeAttributes()[NA_ANGLE];
    angle = angle + 0.05*(RandomNumberGenerator::Instance()->ranf()-0.5)*2*3.147;


    p_new_node->rGetNodeAttributes()[NA_ANGLE] =  angle;
    p_new_node->rGetNodeAttributes()[NA_LENGTH] = 2.0;
    p_new_node->rGetNodeAttributes()[NA_RADIUS] = 0.5;


    // Distribute machines to new cells

    // loop over machines on mother cell

    // Get this cell's type six machine property data
    CellPropertyCollection collection = pParentCell->rGetCellPropertyCollection().template GetProperties<TypeSixMachineProperty>();
    if (collection.GetSize() != 1)
	{
		EXCEPTION("TypeSixMachineCellKiller cannot be used unless each cell has a TypeSixMachineProperty");
	}
	boost::shared_ptr<TypeSixMachineProperty> p_parent_property = boost::static_pointer_cast<TypeSixMachineProperty>(collection.GetProperty());
	std::vector<std::pair<unsigned, double> >& r_parent_data = p_parent_property->rGetMachineData();


	// Get this cell's type six machine property data
	    CellPropertyCollection daughter_collection = pNewCell->rGetCellPropertyCollection().template GetProperties<TypeSixMachineProperty>();
	    if (daughter_collection.GetSize() != 1)
		{
			EXCEPTION("TypeSixMachineCellKiller cannot be used unless each cell has a TypeSixMachineProperty");
		}
		boost::shared_ptr<TypeSixMachineProperty> p_daughter_property = boost::static_pointer_cast<TypeSixMachineProperty>(daughter_collection.GetProperty());
		std::vector<std::pair<unsigned, double> >& r_daughter_data = p_daughter_property->rGetMachineData();



	// Create a new vector to store all pairs less any we might throw away
	std::vector<std::pair<unsigned, double> > new_parent_data;
	std::vector<std::pair<unsigned, double> > new_daughter_data;

	//new_data.reserve(r_data.size() + 1);

	unsigned parent_node_index = this->GetLocationIndexUsingCell(pParentCell);
				Node<DIM>* p_parent_node = this->GetNode(parent_node_index);
				auto cell_centre = p_parent_node->rGetLocation();

				unsigned daughter_node_index = this->GetLocationIndexUsingCell(pNewCell);
								Node<DIM>* p_daughter_node = this->GetNode(daughter_node_index);
								auto daughter_cell_centre = p_daughter_node->rGetLocation();



								// angle defined w.r.t.  centre of cell before it divided
								auto old_cell_centre=0.5*(cell_centre+daughter_cell_centre);

								double cell_angle = p_parent_node->rGetNodeAttributes()[NA_ANGLE];
											double L = 2.0*p_parent_node->rGetNodeAttributes()[NA_LENGTH];
											double R = p_parent_node->rGetNodeAttributes()[NA_RADIUS];

	// Iterate over machines in this cell
	for (auto& r_pair : r_parent_data)
	{







			double theta = r_pair.second;

			// We assume that the machine's angl
			double theta_c = atan2(R, 0.5*L);


			double x_in_cell_frame = DOUBLE_UNSET;
			double y_in_cell_frame = DOUBLE_UNSET;
			if (fabs(theta - 0.5*M_PI) < 0.5*M_PI - theta_c)
			{
				x_in_cell_frame = R/tan(theta);
				y_in_cell_frame = R;
			}
			else if (fabs(theta - 1.5*M_PI) < 0.5*M_PI - theta_c)
			{
				x_in_cell_frame = R/tan(theta);
				y_in_cell_frame = -R;
			}
			else if ((theta > 2*M_PI - theta_c) || (theta < theta_c))
			{
				x_in_cell_frame = (L+sqrt(pow(L,2)-(pow(L,2)-4*pow(R,2))*(1+pow(tan(theta),2))))/(2*(1+pow(tan(theta),2)));
				y_in_cell_frame = x_in_cell_frame*tan(theta);
			}
			else
			{
				x_in_cell_frame = (-L-sqrt(L*L-(L*L-4*R*R)*(1+tan(theta)*tan(theta))))/(2*(1+tan(theta)*tan(theta)));
				y_in_cell_frame = x_in_cell_frame*tan(theta);
			}

			// Compute and store the coordinates of this machine
			c_vector<double, DIM> machine_coords;
			machine_coords[0] = old_cell_centre[0] + x_in_cell_frame*cos(cell_angle) - y_in_cell_frame*sin(cell_angle);
			machine_coords[1] = old_cell_centre[1] + x_in_cell_frame*sin(cell_angle) + y_in_cell_frame*cos(cell_angle);


			if (theta < M_PI/2.0 || theta>3*M_PI/2.0) // machine stays with parent cel;l
			{

				double new_theta = atan2(machine_coords[1]-cell_centre[1], machine_coords[0]-cell_centre[0]);

				r_pair.second=new_theta;
				new_parent_data.emplace_back(std::pair<unsigned, double>(r_pair));


			}
			else //machine moves to daugther cell
			{




				double new_theta = atan2(machine_coords[1]-daughter_cell_centre[1], machine_coords[0]-daughter_cell_centre[0]);
				r_pair.second=new_theta;
				new_daughter_data.emplace_back(std::pair<unsigned, double>(r_pair));

			}

	}

	// assign machine vectors to daughter and mother cells
	r_parent_data=new_parent_data;
	r_daughter_data=new_daughter_data;


    return pNewCellTemp;



}

template<unsigned DIM>
void NodeBasedCellPopulationWithCapsules<DIM>::OutputCellPopulationParameters(out_stream& rParamsFile)
{
    // Currently no specific parameters to output all come from parent classes

    // Call method on direct parent class
    NodeBasedCellPopulation<DIM>::OutputCellPopulationParameters(rParamsFile);
}

// Explicit instantiation
template class NodeBasedCellPopulationWithCapsules<1>;
template class NodeBasedCellPopulationWithCapsules<2>;
template class NodeBasedCellPopulationWithCapsules<3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(NodeBasedCellPopulationWithCapsules)
