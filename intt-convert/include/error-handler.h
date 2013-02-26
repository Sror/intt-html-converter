#pragma once

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace intt { namespace impl {

	struct error_handler_
	{
		template <typename, typename, typename, typename>
		struct result { typedef void type; };

		template <typename Iterator>
		void operator()(Iterator first, Iterator last, Iterator err_pos, qi::info const& what) const
		{
			const size_t contextLength = 25;

			// get context before error
			Iterator start;
			size_t dist = std::distance(first, err_pos);
			if (dist < contextLength)
				start = first;
			else
				start = err_pos - contextLength;
			
			// get context after error
			Iterator end;
			dist = std::distance(err_pos, last);
			if (dist < contextLength)
				end = last;
			else
				end = err_pos + contextLength;

			std::cerr
                << "Error at offset " << (err_pos - first) << " after: \"" << std::string(start, err_pos) << "\"\n"
                << "    Expected: " << what << '\n' 
                << "    Got:      \"" << std::string(err_pos, end) << "\"\n";
		}
	};

	const boost::phoenix::function<error_handler_> err_handler = error_handler_();

}}
