#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../core/no_copy.hpp"
#include "../core/no_move.hpp"

namespace atom::resource
{

class DataLoader : public core::NoCopy, public core::NoMove
{
public:
	DataLoader( const std::wstring& driver_name );
	~DataLoader();

public:
	bool IsValid() const;

public:
	const std::wstring& GetDriverName() const;

	const fs::path& GetPath() const;
	const fs::path& GetDriverPath() const;

protected:
	void CreateInternal();
	void DestroyInternal();

protected:
	bool m_initialized = false;

	std::wstring m_driver_name = { };

	fs::path m_path = { };
	fs::path m_driver_path = { };
};

}