//          __        ____ ____
// ___  ___/  |_ __ _/_   /_   |
// \  \/ /\   __\  |  \   ||   |
//  \   /  |  | |  |  /   ||   |
//   \_/   |__| |____/|___||___|
//
//  License: BSD License ; see LICENSE
//

#ifndef VTU11_VTU11_HPP
#define VTU11_VTU11_HPP

#include "vtu11/inc/alias.hpp"
#include "vtu11/inc/writer.hpp"

#ifdef VTU11_ENABLE_ZLIB
#include "mpi.h"
#endif

namespace vtu11
{

struct Vtu11UnstructuredMesh
{
  const std::vector<double>& points_;
  const std::vector<VtkIndexType>& connectivity_;
  const std::vector<VtkIndexType>& offsets_;
  const std::vector<VtkCellType>& types_;

  const std::vector<double>& points( ){ return points_; }
  const std::vector<VtkIndexType>& connectivity( ){ return connectivity_; }
  const std::vector<VtkIndexType>& offsets( ){ return offsets_; }
  const std::vector<VtkCellType>& types( ){ return types_; }

  size_t numberOfPoints( ){ return points_.size( ) / 3; }
  size_t numberOfCells( ){ return types_.size( ); }
};

struct DistributedVtu11UnstructuredMesh
{
  const std::vector<double>& points_;
  const std::vector<VtkIndexType>& connectivity_;
  const std::vector<VtkIndexType>& offsets_;
  const std::vector<VtkCellType>& types_;

  const std::vector<double>& points( ){ return points_; }
  const std::vector<VtkIndexType>& connectivity( ){ return connectivity_; }
  const std::vector<VtkIndexType>& offsets( ){ return offsets_; }
  const std::vector<VtkCellType>& types( ){ return types_; }

  size_t numberOfPoints( ){
    return computeGlobal(points_.size( ) / 3);
  }

  size_t numberOfCells( ){
    return computeGlobal(types_.size( ));
  }

  size_t computeGlobal(size_t local) {
    int _local = local;
    int global;

    MPI_Reduce(
      &_local,
      &global,
      1,
      MPI_INT,
      MPI_SUM,
      0,
      MPI_COMM_WORLD);
    return static_cast<size_t>(global);
  }
};

//! Helper to compute the offsets for geometries with deterministic number of points
std::vector<VtkIndexType> CopmuteOffsets(const std::vector<VtkCellType>& Types);

/*! Write modes (not case sensitive):
 *
 *  - Ascii
 *  - Base64Inline
 *  - Base64Appended
 *  - RawBinary
 *  - RawBinaryCompressed
 *
 *  Comments:
 *  - RawCompressedBinary needs zlib to be present. If VTU11_ENABLE_ZLIB
 *    is not defined, the uncompressed version is used instead.
 *  - Compressing data takes more time than writing more data uncompressed
 *  - Ascii produces surprisingly small files, is nice to debug, but
 *    is rather slow to read in Paraview. Archiving ascii .vtu files using
 *    a standard zip tool (for example) produces decently small file sizes.
 *  - Writing raw binary data breakes the xml standard. To still produce
 *    valid xml files you can use base64 encoding, at the cost of having
 *    about 30% times larger files.
 *  - Both raw binary modes use appended format
 */

//! Writes single file
template<typename MeshGenerator>
void writeVtu( const std::string& filename,
               MeshGenerator& mesh,
               const std::vector<DataSetInfo>& dataSetInfo,
               const std::vector<DataSetData>& dataSetData,
               const std::string& writeMode = "RawBinaryCompressed" );

//! Creates path/baseName.pvtu and path/baseName directory
void writePVtu( const std::string& path,
                const std::string& baseName,
                const std::vector<DataSetInfo>& dataSetInfo,
                size_t numberOfFiles );

//! Forwards path/baseName.vtu to the writeVtu function
template<typename MeshGenerator>
void writePartition( const std::string& path,
                     const std::string& baseName,
                     MeshGenerator& mesh,
                     const std::vector<DataSetInfo>& dataSetInfo,
                     const std::vector<DataSetData>& dataSetData,
                     size_t fileId,
                     const std::string& writeMode = "RawBinaryCompressed" );

#ifdef VTU11_ENABLE_ZLIB
//! Writes single file
template<typename MeshGenerator>
void writeVtu( const std::string& filename,
               MeshGenerator& mesh,
               const std::vector<DataSetInfo>& dataSetInfo,
               const std::vector<DataSetData>& dataSetData,
               const MPI_Comm MPIComm,
               const std::string& writeMode = "RawBinaryCompressed" );
#endif

} // namespace vtu11

#include "vtu11/impl/vtu11_impl.hpp"

#endif // VTU11_VTU11_HPP
