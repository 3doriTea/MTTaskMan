#pragma once

#define SAFE_DELETE_LIST(list)  \
for (auto& p : list)            \
{                               \
	delete p;                   \
}
