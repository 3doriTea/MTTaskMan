#pragma once

/// <summary>
/// <para>u8string��string�ɕϊ�����B</para>
/// <para>���̂܂܂��ƕ����������邽��UTF8�p��</para>
/// </summary>
/// <param name="_u8string">8u��ړ����ɂ��������񃊃e�����ł�OK</param>
/// <returns>�ϊ����ꂽstring</returns>
static inline std::string ToString(std::u8string _u8string)
{
	return std::string{ _u8string.begin(), _u8string.end() };
}
