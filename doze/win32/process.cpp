#include "process.hpp"
#include "trace.hpp"

// #include "../memory/base.hpp"
#include "../memory/operation.hpp"

namespace atom::win32
{

const TEB* NtCurrentTeb()
{
#if defined( HORIZON_X32 )
	return reinterpret_cast< const TEB* >( __readfsdword( FIELD_OFFSET( NT_TIB, Self ) ) );
#elif defined( HORIZON_X64 )
	return reinterpret_cast< const TEB* >( __readgsqword( FIELD_OFFSET( NT_TIB, Self ) ) );
#else
	return nullptr;
#endif // HORIZON_X32
}

const PEB* NtCurrentPeb()
{
	const auto teb = NtCurrentTeb();

	if( !memory::IsAddressValid( teb ) )
	{
		TRACE( "%s: NtCurrentTeb() error!", __FUNCTION__ );
		return nullptr;
	}

	return teb->ProcessEnvironmentBlock;
}

// 
// [ Process ] implementation
// 
Process::Process()
	: m_nt_status( STATUS_SUCCESS )
	, m_process_id( 0 )
	, m_process( nullptr )
	, m_open( false )
	, m_apc_state()
{ }

Process::Process( std::uint32_t process_id )
	: Process()
{
	const auto nt_status = Create( process_id );

	if( !NT_SUCCESS( nt_status ) )
		TRACE( "%s: Create( 0x%08X ) error! (0x%08X)", __FUNCTION__, process_id, nt_status );
}

Process::~Process()
{
	Destroy();
}

NTSTATUS Process::Create( std::uint32_t process_id )
{
	Destroy();

	m_nt_status = STATUS_SUCCESS;
	m_process_id = process_id;

	if( !m_process_id )
	{
		TRACE( "%s: m_process_id is 0!", __FUNCTION__ );
		return m_nt_status = STATUS_INVALID_PARAMETER;
	}

	m_nt_status = PsLookupProcessByProcessId( HANDLE( m_process_id ), &m_process );

	if( !NT_SUCCESS( m_nt_status ) )
	{
		TRACE( "%s: PsLookupProcessByProcessId( 0x%08X, 0x%016llX ) error! (0x%08X)", __FUNCTION__, m_process_id, std::uint64_t( &m_process ), m_nt_status );
		return m_nt_status;
	}

	return m_nt_status;
}

void Process::Destroy()
{
	Detach();

	if( m_process )
		ObDereferenceObject( m_process );

	m_process_id = 0;
	m_process = nullptr;
}

void Process::Attach()
{
	if( !m_process )
		return;

	if( m_open )
		return;

	m_open = true;
	KeStackAttachProcess( m_process, &m_apc_state );
}

void Process::Detach()
{
	if( !m_open )
		return;

	m_open = false;
	KeUnstackDetachProcess( &m_apc_state );
}

bool Process::IsValid() const
{
	if( !NT_SUCCESS( m_nt_status ) )
		return false;

	if( !m_process_id )
		return false;

	if( !m_process )
		return false;

	return true;
}

NTSTATUS Process::GetStatus() const
{
	return m_nt_status;
}

uint32_t Process::GetProcessId() const
{
	return m_process_id;
}

PEPROCESS Process::GetProcess() const
{
	return m_process;
}

const KAPC_STATE& Process::GetApcState() const
{
	return m_apc_state;
}

}