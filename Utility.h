#pragma once

/// <summary>
/// <para>u8stringをstringに変換する。</para>
/// <para>そのままだと文字化けするためUTF8用に</para>
/// </summary>
/// <param name="_u8string">8uを接頭辞につけた文字列リテラルでもOK</param>
/// <returns>変換されたstring</returns>
static inline std::string ToString(std::u8string _u8string)
{
	return std::string{ _u8string.begin(), _u8string.end() };
}
