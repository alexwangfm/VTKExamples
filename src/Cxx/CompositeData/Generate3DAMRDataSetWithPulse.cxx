// .NAME Generate3DAMRDataSetWithPulse.cxx -- Generated sample 3D AMR dataset
//
// .SECTION Description
//  This utility code generates a simple 3D AMR dataset with a gaussian
//  pulse at the center. The resulting AMR dataset is written using the
//  vtkXMLHierarchicalBoxDataSetWriter.

#include <iostream>
#include <cmath>
#include <sstream>

#include <vtkAMRUtilities.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkOverlappingAMR.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUniformGrid.h>
#include <vtkAMRBox.h>

#include <vtkCompositeDataWriter.h>
#include <vtkHierarchicalBoxDataSet.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkXMLHierarchicalBoxDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLMultiBlockDataWriter.h>

namespace
{
namespace AMRCommon
{
  void WriteUniformGrid( vtkUniformGrid *g, const std::string &prefix );
  void WriteAMRData( vtkOverlappingAMR *amrData, const std::string &prefix );
  vtkHierarchicalBoxDataSet* ReadAMRData( const std::string &file );
  void WriteMultiBlockData( vtkMultiBlockDataSet *mbds, const std::string &prefix );
  vtkUniformGrid* GetGrid( double* origin,double* h,int* ndim );
  void ComputeCellCenter( vtkUniformGrid *grid, const int cellIdx, double c[3] );
}
}
namespace
{
static struct PulseAttributes {
  double origin[3]; // xyz for the center of the pulse
  double width[3];  // the width of the pulse
  double amplitude; // the amplitude of the pulse
} Pulse;

//
// Function prototype declarations
//

// Description:
// Sets the pulse attributes
void SetPulse();

// Description:
// Constructs the vtkHierarchicalBoxDataSet.
vtkOverlappingAMR* GetAMRDataSet();

// Description:
// Attaches the pulse to the given grid.
void AttachPulseToGrid( vtkUniformGrid *grid );
}

//
// Program main
//
int main( int, char *[] )
{
  // STEP 0: Initialize gaussian pulse parameters
  SetPulse();

  // STEP 1: Get the AMR dataset
  vtkOverlappingAMR *amrDataSet = GetAMRDataSet();

  AMRCommon::WriteAMRData( amrDataSet, "Gaussian3D" );
  amrDataSet->Delete();
  return 0;
}
namespace
{
//=============================================================================
//                    Function Prototype Implementation
//=============================================================================

void SetPulse()
{
  Pulse.origin[0] = Pulse.origin[1] = Pulse.origin[2] = -1.0;
  Pulse.width[0]  = Pulse.width[1]  = Pulse.width[2]  = 6.0;
  Pulse.amplitude = 0.0001;
}

//------------------------------------------------------------------------------
void AttachPulseToGrid( vtkUniformGrid *grid )
{
  vtkDoubleArray* xyz = vtkDoubleArray::New( );
  xyz->SetName( "GaussianPulse" );
  xyz->SetNumberOfComponents( 1 );
  xyz->SetNumberOfTuples( grid->GetNumberOfCells() );


  for(int cellIdx=0; cellIdx < grid->GetNumberOfCells(); ++cellIdx )
  {
      double center[3];
      AMRCommon::ComputeCellCenter( grid, cellIdx, center );

      double r = 0.0;
      for( int i=0; i < 3; ++i )
      {
         double dx = center[i]-Pulse.origin[i];
         r += (dx*dx) / (Pulse.width[i]*Pulse.width[i]);
      }
      double f = Pulse.amplitude*std::exp( -r );

      xyz->SetTuple1( cellIdx, f );
  } // END for all cells

  grid->GetCellData()->AddArray( xyz );
  xyz->Delete();
}
//------------------------------------------------------------------------------
vtkOverlappingAMR* GetAMRDataSet()
{
  vtkOverlappingAMR *data = vtkOverlappingAMR::New();
  int blocksPerLevel[2] = {1,3};
  double globalOrigin[3] = {-2.0,-2.0,-2.0};
  data->Initialize(2,blocksPerLevel);
  data->SetOrigin(globalOrigin);
  data->SetGridDescription(VTK_XYZ_GRID);

  double origin[3];
  double h[3];
  int    ndim[3];

  // Root Block -- Block 0
  ndim[0]   = 6; ndim[1]   = ndim[2]   = 5;
  h[0]      = h[1]      = h[2]      = 1.0;
  origin[0] = origin[1] = origin[2] = -2.0;
  int    blockId   = 0;
  int    level     = 0;
  vtkUniformGrid *root = AMRCommon::GetGrid(origin,h,ndim);
  vtkAMRBox box(origin,ndim,h,data->GetOrigin(), data->GetGridDescription());
  AttachPulseToGrid( root );
  data->SetAMRBox(level,blockId,box);
  data->SetDataSet( level, blockId,root);
  root->Delete();

  // Block 1
  ndim[0]   = 3; ndim[1]   = ndim[2]   = 5;
  h[0]      = h[1]      = h[2]      = 0.5;
  origin[0] = origin[1] = origin[2] = -2.0;
  blockId   = 0;
  level     = 1;
  vtkUniformGrid *grid1 = AMRCommon::GetGrid(origin,h,ndim);
  AttachPulseToGrid(grid1);
  vtkAMRBox box1(origin,ndim,h,data->GetOrigin(), data->GetGridDescription());
  data->SetAMRBox(level,blockId,box1);
  data->SetDataSet(level, blockId,grid1);
  grid1->Delete();

  // Block 2
  ndim[0]   = 3; ndim[1]   = ndim[2]   = 5;
  h[0]      = h[1]      = h[2]      = 0.5;
  origin[0] = 0.0; origin[1] = origin[2] = -1.0;
  blockId   = 1;
  level     = 1;
  vtkUniformGrid *grid2 = AMRCommon::GetGrid(origin,h,ndim);
  AttachPulseToGrid(grid2);
  vtkAMRBox box2(origin,ndim,h,data->GetOrigin(), data->GetGridDescription());
  data->SetAMRBox(level,blockId,box2);
  data->SetDataSet(level,blockId,grid2);
  grid2->Delete();

  // Block 3
  ndim[0]   = 3; ndim[1]   = ndim[2]   = 7;
  h[0]      = h[1]      = h[2]      = 0.5;
  origin[0] = 2.0; origin[1] = origin[2] = -1.0;
  blockId   = 2;
  level     = 1;
  vtkUniformGrid *grid3 = AMRCommon::GetGrid(origin,h,ndim);
  vtkAMRBox box3(origin,ndim,h,data->GetOrigin(), data->GetGridDescription());
  AttachPulseToGrid(grid3);
  data->SetAMRBox(level,blockId,box3);
  data->SetDataSet( level, blockId,grid3);
  grid3->Delete();

  vtkAMRUtilities::BlankCells(data);
  return( data );
}
}
namespace
{
namespace AMRCommon {

//------------------------------------------------------------------------------
// Description:
// Writes a uniform grid as a structure grid
void WriteUniformGrid( vtkUniformGrid *g, const std::string &prefix )
{
  vtkXMLImageDataWriter *imgWriter = vtkXMLImageDataWriter::New();

  std::ostringstream oss;
  oss << prefix << "." << imgWriter->GetDefaultFileExtension();
  imgWriter->SetFileName( oss.str().c_str() );
  imgWriter->SetInputData( g );
  imgWriter->Write();

  imgWriter->Delete();
}

//------------------------------------------------------------------------------
// Description:
// Writes the given AMR dataset to a *.vth file with the given prefix.
void WriteAMRData( vtkOverlappingAMR *amrData, const std::string &prefix )
{
  vtkCompositeDataWriter *writer = vtkCompositeDataWriter::New();

  std::ostringstream oss;
  oss << prefix << ".vthb";
  writer->SetFileName(oss.str().c_str());
  writer->SetInputData(amrData);
  writer->Write();
  writer->Delete();
}

//------------------------------------------------------------------------------
// Description:
// Reads AMR data to the given data-structure from the prescribed file.
vtkHierarchicalBoxDataSet* ReadAMRData( const std::string &file )
{
  vtkXMLHierarchicalBoxDataReader *myAMRReader=
   vtkXMLHierarchicalBoxDataReader::New();

  std::ostringstream oss;
  oss.str("");
  oss.clear();
  oss << file << ".vthb";

  std::cout << "Reading AMR Data from: " << oss.str() << std::endl;
  std::cout.flush();

  myAMRReader->SetFileName( oss.str().c_str() );
  myAMRReader->Update();

  vtkHierarchicalBoxDataSet *amrData =
   vtkHierarchicalBoxDataSet::SafeDownCast( myAMRReader->GetOutput() );
  return( amrData );
}

//------------------------------------------------------------------------------
// Description:
// Writes the given multi-block data to an XML file with the prescribed prefix
void WriteMultiBlockData( vtkMultiBlockDataSet *mbds, const std::string &prefix )
{
  // Sanity check
  vtkXMLMultiBlockDataWriter *writer = vtkXMLMultiBlockDataWriter::New();

  std::ostringstream oss;
  oss.str(""); oss.clear();
  oss << prefix << "." << writer->GetDefaultFileExtension();
  writer->SetFileName( oss.str( ).c_str( ) );
  writer->SetInputData( mbds );
  writer->Write();
  writer->Delete();
}

//------------------------------------------------------------------------------
// Constructs a uniform grid instance given the prescribed
// origin, grid spacing and dimensions.
vtkUniformGrid* GetGrid( double* origin,double* h,int* ndim )
{
  vtkUniformGrid *grd = vtkUniformGrid::New();
  grd->Initialize();
  grd->SetOrigin( origin );
  grd->SetSpacing( h );
  grd->SetDimensions( ndim );
  return grd;
}

//------------------------------------------------------------------------------
// Computes the cell center for the cell corresponding to cellIdx w.r.t.
// the given grid. The cell center is stored in the supplied buffer c.
void ComputeCellCenter( vtkUniformGrid *grid, const int cellIdx, double c[3] )
{
  vtkCell *myCell = grid->GetCell( cellIdx );

  double pCenter[3];
  double *weights = new double[ myCell->GetNumberOfPoints() ];
  int subId       = myCell->GetParametricCenter( pCenter );
  myCell->EvaluateLocation( subId,pCenter,c,weights );
  delete [] weights;
}
}
}
