
#include "TypeSixMachineModifier.hpp"
#include "RandomNumberGenerator.hpp"
#include "TypeSixMachineProperty.hpp"
#include "Exception.hpp"
#include "VtkMeshWriter.hpp"
#include "DundeeTypeSixSecretionEnumerations.hpp"
#include "OutputFileHandler.hpp"
#include "NodesOnlyMesh.hpp"

template<unsigned DIM>
TypeSixMachineModifier<DIM>::TypeSixMachineModifier()
    : AbstractCellBasedSimulationModifier<DIM>(),
      mOutputDirectory("")
{
}

template<unsigned DIM>
TypeSixMachineModifier<DIM>::~TypeSixMachineModifier()
{
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::SetOutputDirectory(std::string directory)
{
    mOutputDirectory = directory;
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::UpdateAtEndOfTimeStep(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
    UpdateCellData(rCellPopulation);
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::WriteVtk(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
#ifdef CHASTE_VTK

    // Store the present time as a string
    unsigned num_timesteps = SimulationTime::Instance()->GetTimeStepsElapsed();
    std::stringstream time;
    time << num_timesteps;

    // Create mesh writer for VTK output
    ///\todo remove hardcoding of "machine_results_from_time_0" below
    VtkMeshWriter<DIM, DIM> mesh_writer(mOutputDirectory+"/machine_results_from_time_0",
                                        "machine_results_"+time.str(),
                                        false);

    std::vector<Node<DIM>*> machine_nodes;

    // Create vector to store VTK cell data
    std::vector<double> vtk_machine_data;

    // Iterate over cell population
    unsigned machine_index = 0;
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = rCellPopulation.Begin();
         cell_iter != rCellPopulation.End();
         ++cell_iter)
    { 
        // Get this cell's type six machine property data
        CellPropertyCollection collection = cell_iter->rGetCellPropertyCollection().template GetProperties<TypeSixMachineProperty>();
        if (collection.GetSize() != 1)
        {
            EXCEPTION("TypeSixMachineModifier cannot be used unless each cell has a TypeSixMachineProperty");
        }
        boost::shared_ptr<TypeSixMachineProperty> p_property = boost::static_pointer_cast<TypeSixMachineProperty>(collection.GetProperty());
        std::vector<std::pair<unsigned, double> >& r_data = p_property->rGetMachineData();
        
        // Populate the vector of VTK data
        for (unsigned i=0; i<r_data.size(); i++)
        {
            // Populate data for VTK
            vtk_machine_data.emplace_back((r_data[i]).first);
            
            // Store the location of this machine
            unsigned node_index = rCellPopulation.GetLocationIndexUsingCell(*cell_iter);
            Node<DIM>* p_node = rCellPopulation.GetNode(node_index);
            auto cell_centre = p_node->rGetLocation();
            double cell_angle = p_node->rGetNodeAttributes()[NA_ANGLE];
            double L = p_node->rGetNodeAttributes()[NA_LENGTH];
            double R = p_node->rGetNodeAttributes()[NA_RADIUS];

            double theta = (r_data[i]).second;
            
            // We assume that the machine's angl
            double theta_c = atan2(R, 0.5*L);
            if (theta_c < 0)
            {
                theta_c += 2*M_PI;
            }
            
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
                x_in_cell_frame = (L+sqrt(L*L-(L*L-4*R*R)*(1+tan(theta)*tan(theta))))/(2*(1+tan(theta)*tan(theta)));
                y_in_cell_frame = x_in_cell_frame*tan(theta); 
            }
            else  
            {
            	x_in_cell_frame = (-L-sqrt(L*L-(L*L-4*R*R)*(1+tan(theta)*tan(theta))))/(2*(1+tan(theta)*tan(theta)));
                y_in_cell_frame = x_in_cell_frame*tan(theta); 
            }

            // Compute and store the coordinates of this machine            
            c_vector<double, DIM> machine_coords;
            machine_coords[0] = cell_centre[0] + x_in_cell_frame*cos(cell_angle) - y_in_cell_frame*sin(cell_angle);
            machine_coords[1] = cell_centre[1] + x_in_cell_frame*sin(cell_angle) + y_in_cell_frame*cos(cell_angle);
            
            machine_nodes.push_back(new Node<DIM>(machine_index, machine_coords, false));
            machine_index++;
        }
    }

    mesh_writer.AddPointData("machines", vtk_machine_data); 

    /*
     * At present, the VTK writer can only write things which inherit from AbstractTetrahedralMeshWriter.
     * For now, we do an explicit conversion to NodesOnlyMesh. This can be written to VTK, then visualized
     * as glyphs in Paraview.
     */
    NodesOnlyMesh<DIM> temp_mesh;
    ///\todo Consider removing hardcoding of "1.0" below
    temp_mesh.ConstructNodesWithoutMesh(machine_nodes, 1.0);
    mesh_writer.WriteFilesUsingMesh(temp_mesh);

    *mpVtkMetaFile << "        <DataSet timestep=\"";
    *mpVtkMetaFile << num_timesteps;
    *mpVtkMetaFile << "\" group=\"\" part=\"0\" file=\"results_";
    *mpVtkMetaFile << num_timesteps;
    *mpVtkMetaFile << ".vtu\"/>\n";

    // Tidy up
    for (unsigned i=0; i<machine_nodes.size(); i++)
    {
        delete machine_nodes[i];
    }
#endif //CHASTE_VTK
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::UpdateAtEndOfOutputTimeStep(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
    if (mOutputDirectory == "")
    {
       EXCEPTION("SetOutputDirectory() must be called on a TypeSixMachineModifier before it is passed to a simulation");
    }
    
    WriteVtk(rCellPopulation);
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::SetupSolve(AbstractCellPopulation<DIM,DIM>& rCellPopulation, std::string outputDirectory)
{
///\todo fix writing of .pvd file
#ifdef CHASTE_VTK
    // Create output files for the visualizer
    double time_now = SimulationTime::Instance()->GetTime();
    std::ostringstream time_string;
    time_string << time_now;

    if (mOutputDirectory == "")
    {
        EXCEPTION("SetOutputDirectory() must be called on TypeSixMachineModifier");
    }

    OutputFileHandler output_file_handler(mOutputDirectory +"/machine_results_from_time_" + time_string.str(), false);
    mpVtkMetaFile = output_file_handler.OpenOutputFile("machine_results.pvd");
    *mpVtkMetaFile << "<?xml version=\"1.0\"?>\n";
    *mpVtkMetaFile << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">\n";
    *mpVtkMetaFile << "    <Collection>\n";
#endif //CHASTE_VTK

    WriteVtk(rCellPopulation);

    /*
     * We must update CellData in SetupSolve(), otherwise it will not have been
     * fully initialised by the time we enter the main time loop.
     */
    UpdateCellData(rCellPopulation);
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::UpdateCellData(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
    ///\todo Make sure the cell population is updated?
    //rCellPopulation.Update();

    // Iterate over cell population
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = rCellPopulation.Begin();
         cell_iter != rCellPopulation.End();
         ++cell_iter)
    { 
        // Get this cell's type six machine property data
        CellPropertyCollection collection = cell_iter->rGetCellPropertyCollection().template GetProperties<TypeSixMachineProperty>();
        if (collection.GetSize() != 1)
        {
            EXCEPTION("TypeSixMachineModifier cannot be used unless each cell has a TypeSixMachineProperty");
        }
        boost::shared_ptr<TypeSixMachineProperty> p_property = boost::static_pointer_cast<TypeSixMachineProperty>(collection.GetProperty());
        std::vector<std::pair<unsigned, double> >& r_data = p_property->rGetMachineData();

        ///\todo change parameters to be member variables
        double k_1 = 1.0;
        double k_2 = 1.0;
        double k_3 = 1.0;
        double k_4 = 1.0;
        double k_5 = 1.0;
        double k_6 = 1.0;
        double k_7 = 1.0;
        double k_8 = 1.0;

	    double dt = SimulationTime::Instance()->GetTimeStep();
        assert((k_1 + k_2 + k_3 + k_4 + k_5 + k_6 + k_7 + k_8)*dt <= 1.0);

		// Create a new vector to store all pairs less any we might throw away
		std::vector<std::pair<unsigned, double> > new_data;
		new_data.reserve(r_data.size() + 1);

        for (auto& r_pair : r_data)
        {
	        unsigned old_state = r_pair.first;
	        unsigned new_state = UNSIGNED_UNSET;
		    double r = RandomNumberGenerator::Instance()->ranf();
		
		    switch (old_state)
		    {
		        case 1:
		            if (r < k_2*dt)
		            {
		                new_state = 0;
		            }
		            else if (r < (k_2 + k_3)*dt)
		            {
		                new_state = 2;
		            }
		            break;
		        case 2:
		            if (r < k_4*dt)
		            {
		                new_state = 1;
		            }
		            else if (r < (k_4 + k_5)*dt)
		            {
		                new_state = 3;
		            }
		            break;
		        case 3:
		            if (r < k_6*dt)
		            {
		                new_state = 2;
		            }
		            else if (r < (k_6 + k_7)*dt)
		            {
		                new_state = 4;
		            }
		            break;
		        case 4:
		            if (r < k_8*dt)
		            {
		                new_state = 8;
		            }
		            break;
		    }

		    if (new_state != UNSIGNED_UNSET && new_state > 0)
			{
				r_pair.first = new_state;
			}

		    if (new_state != 0)
		    {
		        new_data.emplace_back(std::pair<unsigned, double>(r_pair));
		    }
		}
		
		// Create a machine?
	    double r = RandomNumberGenerator::Instance()->ranf();
	
        if (r < k_1*dt)
        {
            double theta = 2*M_PI*RandomNumberGenerator::Instance()->ranf();

        	new_data.emplace_back(std::pair<unsigned, double>(1, theta));
        }

		r_data = new_data;
    }
}

template<unsigned DIM>
void TypeSixMachineModifier<DIM>::UpdateAtEndOfSolve(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
#ifdef CHASTE_VTK
    *mpVtkMetaFile << "    </Collection>\n";
    *mpVtkMetaFile << "</VTKFile>\n";
    mpVtkMetaFile->close();
#endif //CHASTE_VTK
}
    
template<unsigned DIM>
void TypeSixMachineModifier<DIM>::OutputSimulationModifierParameters(out_stream& rParamsFile)
{
    AbstractCellBasedSimulationModifier<DIM>::OutputSimulationModifierParameters(rParamsFile);
}

// Explicit instantiation
template class TypeSixMachineModifier<1>;
template class TypeSixMachineModifier<2>;
template class TypeSixMachineModifier<3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(TypeSixMachineModifier)

