#include "TextData.h"

#include <iostream>

//----------------------------------------
void CreateText( const char* raw_text, TextData& out_text_data )
{
	out_text_data.Buffer = raw_text;
	out_text_data.Length = 0;

	while( out_text_data.Buffer[out_text_data.Length] != '\0' )
		++out_text_data.Length;
}

//----------------------------------------
bool AreTextEqual( const char* text1, const TextData& text2 )
{
	uint32_t readCursor = 0;

	while( readCursor < text2.Length && text1[readCursor] != '\0' )
	{
		if( text1[readCursor] != text2.Buffer[readCursor] )
			return false;

		++readCursor;
	}

	return readCursor == text2.Length && text1[readCursor] == '\0';
}

//----------------------------------------
bool AreTextEqual( const TextData& text1, const TextData& text2 )
{
	if( text1.Length != text2.Length )
		return false;

	uint32_t i = 0;

	while( i < text1.Length )
	{
		if( text1.Buffer[i] != text2.Buffer[i] )
			return false;

		++i;
	}

	return true;
}

//----------------------------------------
void PrintTextSlice( const char* buffer, uint32_t from, uint32_t to )
{
	uint32_t i = from;
	while( i < to && buffer[i] != '\0' )
	{
		std::cout << buffer[i];
		++i;
	}
}

//----------------------------------------
void PrintTextSlice( const char* buffer, uint32_t count )
{
	PrintTextSlice( buffer, 0, count );
}

//----------------------------------------
void PrintText( const TextData& text_data )
{
	PrintTextSlice( text_data.Buffer, text_data.Length );
}

//----------------------------------------
void GetLineAndColumn( const TextData& text_data, uint32_t text_position, uint32_t& out_line, uint32_t& out_column )
{
	// #NOTE : cette méthode est pensée pour avoir une correspondance avec les métriques de Notepad++, d'où le fait de démarrer le compte à 1 et de compter 4 pour les tabs

	out_line = 1;
	out_column = 1;
	uint32_t i = 0;

	while( i < text_position )
	{
		if( text_data.Buffer[i] == '\n' )
		{
			++out_line;
			out_column = 1;
		}
		else if( text_data.Buffer[i] == '\t' )
			out_column += 4;							// Pour visualiser correctement dans Notepad++, il faut comptabiliser les colonnes et pas les caractères
		else if( text_data.Buffer[i] != '\r' )
			++out_column;

		++i;
	}
}