//
// Created by student on 06/03/2022.
//

#include <assert.h>
#include "os.h"
int main(int argc, char **argv)
{
    uint64_t pt = alloc_page_frame();

    assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
    page_table_update(pt, 0xcafe, 0xf00d);
    assert(page_table_query(pt, 0xcafe) == 0xf00d);
    page_table_update(pt, 0xcafe, NO_MAPPING);
    assert(page_table_query(pt, 0xcafe) == NO_MAPPING);

    return 0;
}