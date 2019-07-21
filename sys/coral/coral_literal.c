#include "coral.h"
#include <string.h>

void coral_literal_string(coral_literal_t *literal, const char *str)
{
    literal->type = CORAL_LITERAL_TEXT;
    literal->v.as_str.str = str;
    literal->v.as_str.len = strlen(str);
}
// TODO: Implement other literal handling

void coral_literal_int(coral_literal_t *literal, int val)
{
    literal->type = CORAL_LITERAL_INT;
    literal->v.as_int = val;
}
