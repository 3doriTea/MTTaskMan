#pragma once

/// <summary>
/// <para>u8string‚ğstring‚É•ÏŠ·‚·‚éB</para>
/// <para>‚»‚Ì‚Ü‚Ü‚¾‚Æ•¶š‰»‚¯‚·‚é‚½‚ßUTF8—p‚É</para>
/// </summary>
/// <param name="_u8string">8u‚ğÚ“ª«‚É‚Â‚¯‚½•¶š—ñƒŠƒeƒ‰ƒ‹‚Å‚àOK</param>
/// <returns>•ÏŠ·‚³‚ê‚½string</returns>
static inline std::string ToString(std::u8string _u8string)
{
	return std::string{ _u8string.begin(), _u8string.end() };
}
