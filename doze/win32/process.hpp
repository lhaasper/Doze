#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "sdk/teb.hpp"
#include "sdk/peb.hpp"

namespace atom::win32
{

const TEB* NtCurrentTeb();
const PEB* NtCurrentPeb();

// 
// [ Process ]
// 
class Process
{
public:
	Process();
	Process( std::uint32_t process_id );
	~Process();

public:
	NTSTATUS Create( std::uint32_t process_id );
	void Destroy();

public:
	void Attach();
	void Detach();

public:
	bool IsValid() const;

public:
	NTSTATUS GetStatus() const;
	std::uint32_t GetProcessId() const;
	PEPROCESS GetProcess() const;
	const KAPC_STATE& GetApcState() const;

protected:
	NTSTATUS m_nt_status = STATUS_SUCCESS;
	std::uint32_t m_process_id = 0;
	PEPROCESS m_process = nullptr;
	bool m_open = false;
	KAPC_STATE m_apc_state = { };
};

}