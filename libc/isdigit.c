/* implementation of isdigit */

#include <stdlib.h>

/* digits should fall between 0 and 9 */

int isdigit(int c)
{
  return ((c - '0') >= 0 && (c - '0') <= 9) ? 1 : 0;
}
