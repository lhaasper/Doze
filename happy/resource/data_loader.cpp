#define _CRT_SECURE_NO_WARNINGS

#include "data_loader.hpp"

#include "herby.hpp"
#include "intel.hpp"

#include "../win32/console.hpp"
#include "../win32/dialog.hpp"

namespace atom::resource
{

DataLoader::DataLoader( const std::wstring& driver_name )
	: m_initialized( false )
	, m_driver_name( driver_name )
	, m_path()
	, m_driver_path()
{
	CreateInternal();
}

DataLoader::~DataLoader()
{
	DestroyInternal();
}

bool DataLoader::IsValid() const
{
	if( m_driver_name.empty() )
	{
		return false;
	}

	if( !m_initialized )
	{
		return false;
	}

	return true;
}

const std::wstring& DataLoader::GetDriverName() const
{
	return m_driver_name;
}

const fs::path& DataLoader::GetPath() const
{
	return m_path;
}

const fs::path& DataLoader::GetDriverPath() const
{
	return m_driver_path;
}

void DataLoader::CreateInternal()
{
	try
	{
		if( m_driver_name.empty() )
		{
			TRACE( "%s: m_driver_name is not valid!", __FUNCTION__ );
			return;
		}

		if( IsDebuggerPresent() )
		{
			m_path = fs::current_path().parent_path().append( L"bin\\x64" );
		}
		else
		{
			m_path = fs::current_path();
		}

		if( fs::exists( m_path ) )
		{
			m_driver_path = m_path;
			m_driver_path.append( m_driver_name );

			const auto driver_data = reinterpret_cast< const char* >( m_binary_intel.data() );
			const auto driver_size = m_binary_intel.size();

			if( fs::exists( m_driver_path ) )
			{
				if( fs::file_size( m_driver_path ) == driver_size )
				{
					m_initialized = true;
					return;
				}

				if( !fs::remove( m_driver_path ) )
				{
					TRACE( "%s: Couldn't remove '%S' file!", __FUNCTION__, m_driver_path.c_str() );
				}
			}

			std::ofstream stream( m_driver_path, std::ios::binary | std::ios::out );

			if( stream )
			{
				stream.write( driver_data, driver_size );
				stream.close();

				m_initialized = true;
			}
			else
			{
				HORIZON_LOG( "%s: Can't create \"%ws\" file!", __FUNCTION__, m_driver_path.c_str() );
			}
		}
		else
		{
			TRACE( "%s: File '%S' doesn't exist!", __FUNCTION__, m_driver_path.c_str() );
		}
	}
	catch( const std::exception& exception )
	{
		win32::Error( "[Exception]\n%s", exception.what() );
	}
}

void DataLoader::DestroyInternal()
{
	try
	{
		if( fs::exists( m_driver_path ) )
		{
			if( !fs::remove( m_driver_path ) )
			{
				HORIZON_LOG( "%s: Couldn't remove '%S' file!", __FUNCTION__, m_driver_path.c_str() );
			}
		}
		else
		{
			HORIZON_LOG( "%s: File '%S' doesn't exist!", __FUNCTION__, m_driver_path.c_str() );
		}
	}
	catch( const std::exception& exception )
	{
		win32::Error( "[Exception]\n%s", exception.what() );
	}
}

}