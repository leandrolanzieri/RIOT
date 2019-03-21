#include "coral.h"
#include <string.h>

void coral_literal_string(coral_literal_t *literal, char *str)
{
    literal->type = CORAL_LITERAL_TEXT;
    literal->v.as_str.str = str;
    literal->v.as_str.len = strlen(str);
}

