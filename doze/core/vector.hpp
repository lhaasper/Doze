#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::core
{

template< typename T >
class Vector
{
public:
	Vector();
	Vector( const Vector< T >& source );

public:
	~Vector();

public:
	bool empty() const;

	std::size_t size() const;
	std::size_t size_in_bytes() const;
	std::size_t capacity() const;

	T& at( std::size_t index );
	const T& at( std::size_t index ) const;

public:
	void clear();

	T* begin();
	const T* begin() const;

	T* end();
	const T* end() const;

	T& front();
	const T& front() const;

	T& back();
	const T& back() const;

public:
	std::size_t _grow_capacity( std::size_t grow_capacity ) const;

	void resize( std::size_t new_size );
	void resize( std::size_t new_size, const T& element );
	void reserve( std::size_t new_capacity );

public:
	Vector< T >& assign( const Vector< T >& source );

	void push_back( const T& element );
	void pop_back();
	void push_front( const T& element );

	T* insert( const T* iterator, const T& element );

	T* find( const T& element );
	const T* find( const T& element ) const;

public:
	Vector< T >& operator=( const Vector< T >& source );

public:
	T& operator[]( std::size_t index );
	const T& operator[]( std::size_t index ) const;

protected:
	std::size_t m_size = 0;
	std::size_t m_capacity = 0;
	T* m_data = nullptr;
};

template< typename T >
Vector< T >::Vector()
	: m_size( 0 )
	, m_capacity( 0 )
	, m_data( nullptr )
{ }

template< typename T >
Vector< T >::Vector( const Vector< T >& source )
	: Vector()
{
	assign( source );
}

template< typename T >
Vector< T >::~Vector()
{
	clear();
}

template< typename T >
bool Vector< T >::empty() const
{
	return ( m_size == 0 );
}

template< typename T >
std::size_t Vector< T >::size() const
{
	return m_size;
}

template< typename T >
std::size_t Vector< T >::size_in_bytes() const
{
	return ( m_size * sizeof( T ) );
}

template< typename T >
std::size_t Vector< T >::capacity() const
{
	return m_capacity;
}

template< typename T >
T& Vector< T >::at( std::size_t index )
{
	return m_data[ index ];
}

template< typename T >
const T& Vector< T >::at( std::size_t index ) const
{
	return m_data[ index ];
}

template< typename T >
void Vector< T >::clear()
{
	m_size = 0;
	m_capacity = 0;

	if( m_data )
		delete[] m_data;

	m_data = nullptr;
}

template< typename T >
T* Vector< T >::begin()
{
	return m_data;
}

template< typename T >
const T* Vector< T >::begin() const
{
	return m_data;
}

template< typename T >
T* Vector< T >::end()
{
	return ( m_data + m_size );
}

template< typename T >
const T* Vector< T >::end() const
{
	return ( m_data + m_size );
}

template< typename T >
T& Vector< T >::front()
{
	return at( 0 );
}

template< typename T >
const T& Vector< T >::front() const
{
	return at( 0 );
}

template< typename T >
T& Vector< T >::back()
{
	return at( m_size - 1 );
}

template< typename T >
const T& Vector< T >::back() const
{
	return at( m_size - 1 );
}

template< typename T >
std::size_t Vector< T >::_grow_capacity( std::size_t grow_capacity ) const
{
	const auto new_capacity = m_capacity ? ( m_capacity + m_capacity / 2 ) : 8;
	return ( ( new_capacity > grow_capacity ) ? new_capacity : grow_capacity );
}

template< typename T >
void Vector< T >::resize( std::size_t new_size )
{
	if( new_size > m_capacity )
		reserve( _grow_capacity( new_size ) );

	m_size = new_size;
}

template< typename T >
void Vector< T >::resize( std::size_t new_size, const T& element )
{
	if( new_size > m_capacity )
		reserve( _grow_capacity( new_size ) );

	if( new_size > m_size )
	{
		for( std::size_t i = m_size; i < new_size; i++ )
			std::memcpy( &m_data[ i ], &element, sizeof( element ) );

		m_size = new_size;
	}
}

template< typename T >
void Vector< T >::reserve( std::size_t new_capacity )
{
	if( new_capacity <= m_capacity )
		return;

	auto new_data = new T[ new_capacity ];

	if( new_data )
	{
		std::memcpy( new_data, m_data, m_size * sizeof( T ) );
		delete[] m_data;
	}

	m_capacity = new_capacity;
	m_data = new_data;
}

template< typename T >
Vector< T >& Vector< T >::assign( const Vector< T >& source )
{
	if( std::addressof( source ) == this )
		return *this;

	clear();
	resize( source.m_size );

	std::memcpy( m_data, source.m_data, m_size * sizeof( T ) );

	return *this;
}

template< typename T >
void Vector< T >::push_back( const T& element )
{
	if( m_size == m_capacity )
		reserve( _grow_capacity( m_size + 1 ) );

	std::memcpy( &m_data[ m_size ], &element, sizeof( element ) );

	m_size++;
}

template< typename T >
void Vector< T >::pop_back()
{
	if( m_size > 0 )
		m_size--;
}

template< typename T >
void Vector< T >::push_front( const T& element )
{
	if( empty() )
		push_back( element );
	else
		insert( m_data, element );
}

template< typename T >
T* Vector< T >::insert( const T* iterator, const T& element )
{
	const auto displacement = reinterpret_cast< std::ptrdiff_t >( iterator - m_data );

	if( m_size == m_capacity )
		reserve( _grow_capacity( m_size + 1 ) );

	if( displacement < m_size )
		std::memmove( m_data + displacement + 1, m_data + displacement, ( m_size - displacement ) * sizeof( T ) );

	std::memcpy( &m_data[ displacement ], &element, sizeof( element ) );

	m_size++;

	return ( m_data + displacement );
}

template< typename T >
T* Vector< T >::find( const T& element )
{
	const auto data = m_data;
	const auto data_end = m_data + m_size;

	while( data < data_end )
	{
		if( ( *data ) == element )
			break;

		data++;
	}

	return data;
}

template< typename T >
const T* Vector< T >::find( const T& element ) const
{
	const auto data = m_data;
	const auto data_end = m_data + m_size;

	while( data < data_end )
	{
		if( ( *data ) == element )
			break;

		data++;
	}

	return data;
}

template< typename T >
Vector< T >& Vector< T >::operator=( const Vector< T >& source )
{
	return assign( source );
}

template< typename T >
T& Vector< T >::operator[]( std::size_t index )
{
	return at( index );
}

template< typename T >
const T& Vector< T >::operator[]( std::size_t index ) const
{
	return at( index );
}

}