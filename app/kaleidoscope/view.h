#pragma once
#include <memory>

class window
{
public:
	window();
	~window();

public:
	auto run() -> void;

private:
	struct impl;
	std::unique_ptr<impl> m;
};
