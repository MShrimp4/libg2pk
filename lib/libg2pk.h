#ifndef __LIB_G2PK_H__
#define __LIB_G2PK_H__


namespace G2PK
{
  class G2K
  {
  public:
    G2K ();
    ~G2K ();

    std::string    convert      (std::string input,
                                 bool        debug = false);

    std::u32string decompose    (const std::u32string& str);
    std::u32string to_syllables (const std::u32string& str);

  private:

    void *convert_eng_trie; //AC_TRIE_t
    void *batchim_onset_trie; //AC_TRIE_t

    void *tagger; //MeCab::tagger

    std::string convert_english                    (const std::string& str);
    std::string annotate                           (const std::string& str);
    std::string convert_num                        (const std::string& str);
    std::string parse_idioms                       (const std::string& str);
    std::string apply_special_pronounciation_rules (const std::string& str);
    std::string link                               (const std::string& str);
    std::string batchim_onset                      (const std::string& str);

  };

  std::string u32_to_u8 (const std::u32string& str);
  std::u32string u8_to_u32 (const std::string& str);
}



#endif /* __LIB_G2PK_H__ */
