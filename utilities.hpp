/**
 * @file utilities.hpp
 * @author Ulrich Buettemeier
 * @version v0.0.1
 * @date 2021-09-12
 */

#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <iostream>

class util {
public:
    static std::string ltrim(std::string s, std::string const& delim = " \t\r\n");
    static std::string rtrim(std::string s, std::string const& delim = " \t\r\n");   
    static std::string getFileExt(const std::string& s);
    static void check_file (std::string fn);    // Zur Zeit hat die Funktion noch kein Verwendung !
};

/*******************************************************************
 * @brief   Funktion entfernt führende Leerzeichen
 */
std::string util::ltrim(std::string s, std::string const& delim)
{
  return s.erase(0, s.find_first_not_of(delim.c_str()));
}

/*******************************************************************
 * @brief   Funktion entfernt anhängende Leerzeichen
 */
std::string util::rtrim(std::string s, std::string const& delim)
{
    std::string::size_type last = s.find_last_not_of(delim.c_str());
    return last == std::string::npos ? "" : s.erase(last + 1);
}

/*********************************************************************
 *  @brief  ermittelt die Fileextension
 *  @return Extension oder Leerstring
 */
std::string util::getFileExt(const std::string& s)
{
    size_t i = s.rfind('.', s.length());
    if (i != std::string::npos) 
        return(s.substr(i+1, s.length() - i));

    return("");
}

/*********************************************************************
 * @brief   prüft, ob eine '.' vorhanden ist !
 *          Zur Zeit hat die Funktion noch kein Verwendung !
 */
void util::check_file (std::string fn)
{
    int i = fn.find_last_of(".");   // kein Punkt == -1
    std::cout << i << " | " << fn.length() <<  std::endl;

    if (i >= 0) {
        std::cout << "Yes..." << std::endl;
    } else {
        std::cout << "No..." << std::endl;
    }
}

#endif