#include <string>
#include <iostream>
#include <libg2pk.h>



int
main (int argc, char **argv)
{
  G2PK::G2K g2pk = G2PK::G2K ();
  while (true)
    {
      std::string input;

      std::cout << "Input:";
      std::getline (std::cin, input);
      std::cout << "Output: " << g2pk.convert (input) << std::endl;
    }
}
