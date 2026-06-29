#pragma once

#include <numeric>

//----------------------------------------
struct TextData
{
	const char* Buffer{ nullptr };
	uint32_t Length{ 0 };
};

void CreateText( const char* raw_text, TextData& out_text_data );

bool AreTextEqual( const char* text1, const TextData& text2 );
bool AreTextEqual( const TextData& text1, const TextData& text2 );

void PrintTextSlice( const char* buffer, uint32_t from, uint32_t to );
void PrintTextSlice( const char* buffer, uint32_t count );
void PrintText( const TextData& text_data );

void GetLineAndColumn( const TextData& text_data, uint32_t text_position, uint32_t& out_line, uint32_t& out_column );