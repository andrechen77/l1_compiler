#pragma once

#include <L1.h>
#include <memory>

namespace L1{
	std::unique_ptr<Program> parse_file (char *fileName);
}
