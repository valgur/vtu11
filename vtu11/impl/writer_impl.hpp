//          __        ____ ____
// ___  ___/  |_ __ _/_   /_   |
// \  \/ /\   __\  |  \   ||   |
//  \   /  |  | |  |  /   ||   |
//   \_/   |__| |____/|___||___|
//
//  License: BSD License ; see LICENSE
//

#ifndef VTU11_WRITER_IMPL_HPP
#define VTU11_WRITER_IMPL_HPP

#include "inc/utilities.hpp"

#include <fstream>

namespace vtu11
{
namespace detail
{

template<typename T> constexpr const char* formatStr( );

template<> constexpr const char* formatStr<double>( ) { return "%g"; }
template<> constexpr const char* formatStr<long long int>( ) { return "%lld"; }
template<> constexpr const char* formatStr<long int     >( ) { return "%ld"; }
template<> constexpr const char* formatStr<int          >( ) { return "%d"; }
template<> constexpr const char* formatStr<short        >( ) { return "%hd"; }
template<> constexpr const char* formatStr<char         >( ) { return "%hhd"; }
template<> constexpr const char* formatStr<unsigned long long int>( ) { return "%llu"; }
template<> constexpr const char* formatStr<unsigned long int     >( ) { return "%ld"; }
template<> constexpr const char* formatStr<unsigned int          >( ) { return "%d"; }
template<> constexpr const char* formatStr<unsigned short        >( ) { return "%hd"; }
template<> constexpr const char* formatStr<unsigned char         >( ) { return "%hhd"; }

} // namespace detail

template<typename T>
inline void AsciiWriter::writeData( std::ostream& output,
                                    const std::vector<T>& data )
{
    char buffer[64];

    for( auto value : data )
    {
        std::snprintf( buffer, sizeof( buffer ), detail::formatStr<T>( ), value );

        output << buffer << " ";
    }

    output << "\n";
}

template<>
inline void AsciiWriter::writeData( std::ostream& output,
                                    const std::vector<std::int8_t>& data )
{
  for( auto value : data )
  {
    // will otherwise interpret uint8 as char and output nonsense instead
  	// changed the datatype from unsigned to int
      output << static_cast<int>( value ) << " ";
  }

  output << "\n";
}

inline void AsciiWriter::writeAppended( std::ostream& )
{

}

inline void AsciiWriter::addHeaderAttributes( StringStringMap& )
{
}

inline void AsciiWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "ascii";
}

inline StringStringMap AsciiWriter::appendedAttributes( )
{
  return { };
}

// ----------------------------------------------------------------

template<typename T>
inline void Base64BinaryWriter::writeData( std::ostream& output,
                                           const std::vector<T>& data )
{
  HeaderType numberOfBytes = data.size( ) * sizeof( T );

  output << base64Encode( &numberOfBytes, &numberOfBytes + 1 );
  output << base64Encode( data.begin( ), data.end( ) );

  output << "\n";
}

inline void Base64BinaryWriter::writeAppended( std::ostream& )
{

}

inline void Base64BinaryWriter::addHeaderAttributes( StringStringMap& attributes )
{
  attributes["header_type"] = dataTypeString<HeaderType>( );
}

inline void Base64BinaryWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "binary";
}

inline StringStringMap Base64BinaryWriter::appendedAttributes( )
{
  return { };
}

// ----------------------------------------------------------------

template<typename T>
inline void Base64BinaryAppendedWriter::writeData( std::ostream&,
                                                   const std::vector<T>& data )
{
  HeaderType rawBytes = data.size( ) * sizeof( T );

  appendedData.emplace_back( reinterpret_cast<const char*>( &data[0] ), rawBytes );

  offset += encodedNumberOfBytes( rawBytes + sizeof( HeaderType ) );
}

inline void Base64BinaryAppendedWriter::writeAppended( std::ostream& output )
{
  for( auto dataSet : appendedData )
  {
    // looks like header and data has to be encoded at once
    std::vector<char> data( dataSet.second + sizeof( HeaderType ) );

    *reinterpret_cast<HeaderType*>( &data[0] ) = dataSet.second;

    std::copy( dataSet.first, dataSet.first + dataSet.second, &data[ sizeof( HeaderType ) ] );

    output << base64Encode( data.begin( ), data.end( ) );
  }

  output << "\n";
}

inline void Base64BinaryAppendedWriter::addHeaderAttributes( StringStringMap& attributes )
{
  attributes["header_type"] = dataTypeString<HeaderType>( );
}

inline void Base64BinaryAppendedWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "appended";
  attributes["offset"] = std::to_string( offset );
}

inline StringStringMap Base64BinaryAppendedWriter::appendedAttributes( )
{
  return { { "encoding", "base64" } };
}

// ----------------------------------------------------------------

template<typename T>
inline void RawBinaryAppendedWriter::writeData( std::ostream&,
                                                const std::vector<T>& data )
{
  HeaderType rawBytes = data.size( ) * sizeof( T );

  appendedData.emplace_back( reinterpret_cast<const char*>( &data[0] ), rawBytes );

  offset += sizeof( HeaderType ) + rawBytes;
}

inline void RawBinaryAppendedWriter::writeAppended( std::ostream& output )
{
  for( auto dataSet : appendedData )
  {
    const char* headerBegin = reinterpret_cast<const char*>( &dataSet.second );

    for( const char* ptr = headerBegin; ptr < headerBegin + sizeof( HeaderType ); ++ptr )
    {
      output << *ptr;
    }

    for( const char* ptr = dataSet.first; ptr < dataSet.first + dataSet.second; ++ptr )
    {
      output << *ptr;
    }
  }

  output << "\n";
}

inline void RawBinaryAppendedWriter::addHeaderAttributes( StringStringMap& attributes )
{
  attributes["header_type"] = dataTypeString<HeaderType>( );
}

inline void RawBinaryAppendedWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "appended";
  attributes["offset"] = std::to_string( offset );
}

inline StringStringMap RawBinaryAppendedWriter::appendedAttributes( )
{
  return { { "encoding", "raw" } };
}

} // namespace vtu11

#endif // VTU11_WRITER_IMPL_HPP
