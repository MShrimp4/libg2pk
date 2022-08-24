#include <algorithm>
#include <array>
#include <cctype>

//I/O
#include <iostream>
#include <fstream>
#include <filesystem>

//String
#include <locale>
#include <codecvt>
#include <cstring>
#include <string>
#include <regex>

//Other libs
#include <ahocorasick.h>
#include <hangul.h>
#include <mecab.h>

#include "libg2pk.h"
#include "libg2pk_conf.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace G2PK
{

static void listener (AC_TEXT_t *text, void *user);
static AC_TRIE_t *fill_trie (const std::string dict_dir);



static void listener (AC_TEXT_t *text, void *user)
{
  std::string *str = (std::string *)user;
  (*str).assign (text->astring, text->length);
}



static AC_TRIE_t *
fill_trie (const std::string dict_dir)
{
  AC_TRIE_t *trie = ac_trie_create ();

  std::ifstream dict_input (dict_dir);
  for (std::string line; getline (dict_input, line);)
    {
      size_t token_pos = line.find ("\t");
      if (token_pos == std::string::npos)
        continue;
      std::string pattern = line.substr (0, token_pos);
      std::string replace = line.substr (token_pos + 1);

      //std::cout << "{" << pattern << " , " << replace << "}" << std::endl;

      AC_PATTERN_t ac_pattern = {{pattern.data(), pattern.length()},
                                 {replace.data(), replace.length()},
                                 {{0},AC_PATTID_TYPE_DEFAULT}};

      ac_trie_add (trie, &ac_pattern, 1);
    }
  dict_input.close();

  ac_trie_finalize (trie);

  return trie;
}



std::string
u32_to_u8 (const std::u32string& str)
{
  std::wstring_convert <std::codecvt_utf8<char32_t>, char32_t> wconv_from32;

  return wconv_from32.to_bytes(str.data());
}



std::u32string
u8_to_u32 (const std::string& str)
{
  std::wstring_convert <std::codecvt_utf8<char32_t>, char32_t> wconv_to32;

  return wconv_to32.from_bytes(str.data());
}



/***********************
 * Class methods
 ***********************/



G2K::G2K ()
{
  std::filesystem::path libexecdir (TOSTRING(libg2pk_LIBEXECDIR));
  std::filesystem::path english ("libg2pk/english.txt");
  std::filesystem::path batchim ("libg2pk/batchim.txt");
  convert_eng_trie   = (void *) fill_trie (libexecdir / english);
  batchim_onset_trie = (void *) fill_trie (libexecdir / batchim);
  tagger = (void *) MeCab::createTagger("");

  if (!convert_eng_trie || !batchim_onset_trie || !tagger)
    throw std::runtime_error("G2Pk init failed\n");
}



G2K::~G2K ()
{
  ac_trie_release ((AC_TRIE_t *)convert_eng_trie);
  ac_trie_release ((AC_TRIE_t *)batchim_onset_trie);
  delete (MeCab::Tagger *)tagger;
}



std::string
G2K::convert (std::string input, bool debug)
{
  std::string& str = input;

  str = convert_english (str);
  if (debug)
    std::cout << "Covert English :" << str << std::endl;
  str = parse_idioms (str);
  if (debug)
    std::cout << "Parse Idioms :" << str << std::endl;
  str = annotate (str);
  if (debug)
    std::cout << "Annotate :" << str << std::endl;
  str = u32_to_u8(decompose (u8_to_u32(str)));
  if (debug)
    std::cout << "Decompose :" << str << std::endl;
  str = convert_num (str);
  if (debug)
    std::cout << "Convert Numbers :" << str << std::endl;
  str = apply_special_pronounciation_rules (str);
  if (debug)
    std::cout << "Apply Special :" << str << std::endl;
  str = link (str);
  if (debug)
    std::cout << "Batchim + Onset :" << str << std::endl;
  str = batchim_onset (str);
  if (debug)
    std::cout << "Link :" << str << std::endl;
  str = u32_to_u8 (to_syllables (u8_to_u32(str)));
  if (debug)
    std::cout << "To Syllables:" << str << std::endl;
  str.pop_back();

  return str;
}



std::string
G2K::convert_english (const std::string& str)
{
  std::string input = str;

  std::transform(input.begin(), input.end(), input.begin(),
                 [](unsigned char c){ return std::tolower(c); });

  AC_TEXT_t ac_input = {input.data(), input.length()};

  std::string output = "";
  output.reserve(str.size());
  multifast_replace ((AC_TRIE_t *)convert_eng_trie, 
                     &ac_input, MF_REPLACE_MODE_NORMAL,
                     listener, (void *)&output);
  multifast_rep_flush ((AC_TRIE_t *)convert_eng_trie, 0);

  return output;
}



std::string
G2K::annotate (const std::string& str)
{
  const char *input = str.c_str();
  std::string result = "";

  MeCab::Tagger *mecab_tagger = (MeCab::Tagger *)tagger;
  const MeCab::Node* node = mecab_tagger->parseToNode(input);
  if (!node)
    return str;

  size_t last_pos = 0;
  for (; node; node = node->next) {
    if (node->stat == MECAB_BOS_NODE)
      ;
    else if (node->stat == MECAB_EOS_NODE)
      result.append("$");
    else
      {
        size_t copy_end = (node->surface - input) + node->length;
        result.append (input, last_pos, copy_end - last_pos);
        last_pos = copy_end;

        const char *pos_start = strchr (node->feature, '+');
        const char *pos_end   = strchr (node->feature, ',');
        if (pos_start == NULL || pos_start >= pos_end)
          pos_start = node->feature;
        else
          pos_start++;

        if (strchr ("VJEB", *pos_start) != NULL)
          {
            result.push_back ('/');
            result.push_back (*pos_start);
          }
      }
  }

  delete node;

  return result;
}



std::string
G2K::convert_num (const std::string& str)
{
  std::string output = str;
  return output;
}



std::string
G2K::parse_idioms (const std::string& str)
{
  std::string output = str;
  return output;
}



std::u32string
G2K::decompose (const std::u32string& str)
{
  std::u32string output     = U"";
  output.reserve(str.size()*3);

  for (const char32_t &c: str)
    {
      if (hangul_is_syllable ((ucschar) c))
        {
          ucschar cho, jung, jong;
          hangul_syllable_to_jamo ((ucschar) c, &cho, &jung, &jong);
          output.push_back ((char32_t) cho);
          output.push_back ((char32_t) jung);
          if (jong != 0)
            output.push_back ((char32_t) jong);
        }
      else
        {
          output.push_back (c);
        }
    }
  return output;
}



std::string
G2K::apply_special_pronounciation_rules (const std::string& str)
{
  static const std::array <std::array <std::string, 2>, 45> regex_str =
    {{
      {u8"([ᄀᄁᄃᄄㄹᄆᄇᄈᄌᄍᄎᄏᄐᄑᄒ])ᅨ", u8"$1ᅦ"},
      {u8"의/J", u8"에"},
      {u8"(\\sᄋ)ᅴ", u8"$1ᅵ"},
      {u8"([ᄌᄍᄎ])ᅧ", u8"$1ᅥ"},
      {u8"([ᄀᄁᄂᄃᄄᄅᄆᄇᄈᄉᄊᄌᄍᄎᄏᄐᄑᄒ])ᅴ", u8"$1ᅵ"},
      {u8"([그])ᆮᄋ", u8"$1ᄉ"},
      {u8"([으])[ᆽᆾᇀᇂ]ᄋ", u8"$1ᄉ"},
      {u8"([으])[ᆿ]ᄋ", u8"$1ᄀ"},
      {u8"([으])[ᇁ]ᄋ", u8"$1ᄇ"},
      {u8"ᆰ/V([ᄀᄁ])", u8"ᆯᄁ"},
      {u8"([ᆲᆴ])/Vᄀ", u8"$1ᄁ"},
      {u8"([ᆲᆴ])/Vᄃ", u8"$1ᄄ"},
      {u8"([ᆲᆴ])/Vᄉ", u8"$1ᄊ"},
      {u8"([ᆲᆴ])/Vᄌ", u8"$1ᄍ"},
      {u8"([ᆫᆷ])/Vᄀ", u8"$1ᄁ"},
      {u8"([ᆫᆷ])/Vᄃ", u8"$1ᄄ"},
      {u8"([ᆫᆷ])/Vᄉ", u8"$1ᄊ"},
      {u8"([ᆫᆷ])/Vᄌ", u8"$1ᄍ"},
      {u8"ᆬ/Vᄀ", u8"ᆫᄁ"},
      {u8"ᆬ/Vᄃ", u8"ᆫᄄ"},
      {u8"ᆬ/Vᄉ", u8"ᆫᄊ"},
      {u8"ᆬ/Vᄌ", u8"ᆫᄍ"},
      {u8"ᆱ/Vᄀ", u8"ᆷᄁ"},
      {u8"ᆱ/Vᄃ", u8"ᆷᄄ"},
      {u8"ᆱ/Vᄉ", u8"ᆷᄊ"},
      {u8"ᆱ/Vᄌ", u8"ᆷᄍ"},
      {u8"(바)ᆲ($|[^ᄋᄒ])", u8"$1ᆸ$2"},
      {u8"(너)ᆲ([ᄌᄍ]ᅮ|[ᄃᄄ]ᅮ)", u8"$1ᆸ$2"},
      {u8"ᆮᄋ([ᅵᅧ])", u8"ᄌ$1"},
      {u8"ᇀᄋ([ᅵᅧ])", u8"ᄎ$1"},
      {u8"ᆴᄋ([ᅵᅧ])", u8"ᆯᄎ$1"},
      {u8"ᆮᄒ([ᅵ])", u8"ᄎ$1"},
      {u8"ᆯ/E ᄀ", u8"ᆯ ᄁ"},
      {u8"ᆯ/E ᄃ", u8"ᆯ ᄄ"},
      {u8"ᆯ/E ᄇ", u8"ᆯ ᄈ"},
      {u8"ᆯ/E ᄉ", u8"ᆯ ᄊ"},
      {u8"ᆯ/E ᄌ", u8"ᆯ ᄍ"},
      {u8"ᆯ걸", u8"ᆯ껄"},
      {u8"ᆯ밖에", u8"ᆯ빠께"},
      {u8"ᆯ세라", u8"ᆯ쎄라"},
      {u8"ᆯ수록", u8"ᆯ쑤록"},
      {u8"ᆯ지라도", u8"ᆯ찌라도"},
      {u8"ᆯ지언정", u8"ᆯ찌언정"},
      {u8"ᆯ진대", u8"ᆯ찐대"},
      {u8"/[VJEB]", u8""}
      }};

  std::string output = str;
  output.reserve(str.size());

  for (const std::array <std::string, 2> &item : regex_str)
    {
      std::regex regex (item[0]);
      std::string replace = item[1];
      output = std::regex_replace (output, regex, replace);
    }

  return output;
}



std::string
G2K::batchim_onset (const std::string& str)
{
  std::string input = str;

  AC_TEXT_t ac_input = {input.data(), input.length()};

  std::string output = "";
  output.reserve(str.size());
  multifast_replace ((AC_TRIE_t *)batchim_onset_trie, 
                     &ac_input, MF_REPLACE_MODE_NORMAL,
                     listener, (void *)&output);
  multifast_rep_flush ((AC_TRIE_t *)batchim_onset_trie, 0);

  return output;
}



std::string
G2K::link (const std::string& str)
{
  static const std::array <std::array <std::string, 2>, 49> regex_str =
    {{
        {u8"ᆨᄋ", u8"ᄀ"},
        {u8"ᆩᄋ", u8"ᄁ"},
        {u8"ᆫᄋ", u8"ᄂ"},
        {u8"ᆮᄋ", u8"ᄃ"},
        {u8"ᆯᄋ", u8"ᄅ"},
        {u8"ᆷᄋ", u8"ᄆ"},
        {u8"ᆸᄋ", u8"ᄇ"},
        {u8"ᆺᄋ", u8"ᄉ"},
        {u8"ᆻᄋ", u8"ᄊ"},
        {u8"ᆽᄋ", u8"ᄌ"},
        {u8"ᆾᄋ", u8"ᄎ"},
        {u8"ᆿᄋ", u8"ᄏ"},
        {u8"ᇀᄋ", u8"ᄐ"},
        {u8"ᇁᄋ", u8"ᄑ"},
        {u8"ᆪᄋ", u8"ᆨᄊ"},
        {u8"ᆬᄋ", u8"ᆫᄌ"},
        {u8"ᆰᄋ", u8"ᆯᄀ"},
        {u8"ᆱᄋ", u8"ᆯᄆ"},
        {u8"ᆲᄋ", u8"ᆯᄇ"},
        {u8"ᆳᄋ", u8"ᆯᄊ"},
        {u8"ᆴᄋ", u8"ᆯᄐ"},
        {u8"ᆵᄋ", u8"ᆯᄑ"},
        {u8"ᆹᄋ", u8"ᆸᄊ"},
        {u8"ᆨ ᄋ", u8" ᄀ"},
        {u8"ᆩ ᄋ", u8" ᄁ"},
        {u8"ᆫ ᄋ", u8" ᄂ"},
        {u8"ᆮ ᄋ", u8" ᄃ"},
        {u8"ᆯ ᄋ", u8" ᄅ"},
        {u8"ᆷ ᄋ", u8" ᄆ"},
        {u8"ᆸ ᄋ", u8" ᄇ"},
        {u8"ᆺ ᄋ", u8" ᄉ"},
        {u8"ᆻ ᄋ", u8" ᄊ"},
        {u8"ᆽ ᄋ", u8" ᄌ"},
        {u8"ᆾ ᄋ", u8" ᄎ"},
        {u8"ᆿ ᄋ", u8" ᄏ"},
        {u8"ᇀ ᄋ", u8" ᄐ"},
        {u8"ᇁ ᄋ", u8" ᄑ"},
        {u8"ᆪ ᄋ", u8"ᆨ ᄊ"},
        {u8"ᆬ ᄋ", u8"ᆫ ᄌ"},
        {u8"ᆰ ᄋ", u8"ᆯ ᄀ"},
        {u8"ᆱ ᄋ", u8"ᆯ ᄆ"},
        {u8"ᆲ ᄋ", u8"ᆯ ᄇ"},
        {u8"ᆳ ᄋ", u8"ᆯ ᄊ"},
        {u8"ᆴ ᄋ", u8"ᆯ ᄐ"},
        {u8"ᆵ ᄋ", u8"ᆯ ᄑ"},
        {u8"ᆹ ᄋ", u8"ᆸ ᄊ"},
        {u8"ᇂᄋ", u8"ᄋ"},
        {u8"ᆭᄋ", u8"ᄂ"},
        {u8"ᆶᄋ", u8"ᄅ"}
      }};

  std::string output = str;

  for (const std::array <std::string, 2> &item : regex_str)
    {
      std::regex regex (item[0]);
      std::string replace = item[1];
      output = std::regex_replace (output, regex, replace);
    }

  return output;
}



std::u32string
G2K::to_syllables (const std::u32string& str)
{
  char32_t output [str.size() + 10];

  int n = hangul_jamos_to_syllables
    ((ucschar *) output, str.size() + 10,
     (ucschar *) str.data(),    str.size());
  output[n] = U'\0';

  return std::u32string(output);
}

} //namespace G2PK
