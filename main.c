#include <stdio.h>
#include "pocky.h"
int main()
{
	struct pocky_base *base = pocky_init();
    pocky_add_ev(1, base, NULL, NULL);
    pocky_add_ev(1, base, NULL, NULL);
    pocky_add_ev(2, base, NULL, NULL);
    pocky_add_ev(3, base, NULL, NULL);
    pocky_add_ev(4, base, NULL, NULL);
    pocky_add_ev(5, base, NULL, NULL);
    pocky_accept_ev(4, 1, base, NULL, NULL);
    printf(" %u element in base\n", pocky_base_size(base));

    sample_trigger(base, 4);
    pocky_destroy_base(base);
	return 0;
}
