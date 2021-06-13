#include "table_processor.hpp"

#include <iomanip>
#include <iostream>

void table_processor_base::out_iterations() const
{
	if (!m_suppress_output)
	{
		std::cout << std::setw(16) << m_iterations << std::string(16, '\b') << std::flush;
	}
}

void table_processor_base::out_calculating_started(const std::string& message) const
{
	if (!m_suppress_output)
	{
		std::cout << "Calculating for " << std::setw(18) << std::setiosflags(std::ios::left)
				  << message << ": " << std::resetiosflags(std::ios::left) << std::flush;
	}
}

void table_processor_base::out_calculating_fineshed(std::chrono::microseconds::rep microseconds_passed) const
{
	if (!m_suppress_output)
	{
		double ips {static_cast<double>(iterations()) / static_cast<double>(microseconds_passed)};
		std::cout << "took " << (microseconds_passed / 1000) << " milliseconds ("
				  << m_iterations << " iteration(s), "
				  << m_reused << " reused; "
				  << ips << " Mips)" << std::endl;
	}
}
