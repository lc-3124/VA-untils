/*
 * (c) 2025 Lc3124
 * License (MIT)
 * 这是一个工具类，包括中文转拼音等
 *
 */
#ifndef _VAUNTILS_HPP_
#define _VAUNTILS_HPP_

#include <string>
#include <vector>

class VaUntils {
private:
public:
    static bool Char2Pinyin(const std::string resource, std::string& Pinying);
    static bool Char2Pinyin2(const std::string resource,std::vector<std::string> &Pinyin);
};

#endif
