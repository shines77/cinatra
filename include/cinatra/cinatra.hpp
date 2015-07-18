
#pragma once

#include "http_server.hpp"
#include "http_router.hpp"
#include "request.hpp"
#include "response.hpp"
#include "logging.hpp"
#include "aop.hpp"

#include "check_login_aspect.hpp"

#include <string>
#include <vector>


namespace cinatra
{
	class Cinatra
	{
	public:
		template<typename...Args>
		Cinatra& route(Args&&... args)
		{
			router_.route(std::forward<Args>(args)...);
			return *this;
		}
		Cinatra& threads(int num)
		{
			num_threads_ = num < 1 ? 1 : num;
			return *this;
		}

		Cinatra& error_handler(error_handler_t error_handler)
		{
			error_handler_ = error_handler;
			return *this;
		}

		Cinatra& listen(const std::string& address, const std::string& port)
		{
			listen_addr_ = address;
			listen_port_ = port;
			return *this;
		}

		Cinatra& listen(const std::string& address, unsigned short port)
		{
			listen_addr_ = address;
			listen_port_ = boost::lexical_cast<std::string>(port);
			return *this;
		}

		Cinatra& public_dir(const std::string& dir)
		{
			public_dir_ = dir;
			return *this;
		}

		void init(const Request& req, Response& res)
		{
			req_ = &req;
			res_ = &res;
		}

		void run()
		{
			HTTPServer s(num_threads_);
			s.set_init_handler(std::bind(&Cinatra::init, this, std::placeholders::_1, std::placeholders::_2));
			s.set_request_handler([this](const Request& req, Response& res)
			{
				return invoke<CheckLoginAspect>(res, &Cinatra::dispatch, this, req, res);
			})
				.set_error_handler([this](int code, const std::string& msg, const Request& req, Response& res)
			{
				LOG_DBG << "Handle error:" << code << " " << msg << " with path " << req.path();
				if (error_handler_
					&& error_handler_(code,msg,req,res))
				{
					return true;
				}

				LOG_DBG << "In defaule error handler";
				std::string html;
				auto s = status_header(code);
				html = "<html><head><title>" + s.second + "</title></head>";
				html += "<body>";
				html += "<h1>" + boost::lexical_cast<std::string>(s.first) + " " + s.second + " " + "</h1>";
				if (!msg.empty())
				{
					html += "<br> <h2>Message: " + msg + "</h2>";
				}
				html += "</body></html>";

				res.set_status_code(s.first);

				res.write(html);

				return true;
			})
				.public_dir(public_dir_)
				.listen(listen_addr_, listen_port_)
				.run();
		}
	private:
		bool dispatch(const Request& req, Response& res)
		{
			return router_.dispatch(req, res);
		}
	private:
		int num_threads_ = std::thread::hardware_concurrency();
		std::string listen_addr_;
		std::string listen_port_;
		std::string public_dir_;

		error_handler_t error_handler_;

		HTTPRouter router_;
		const Request* req_;
		Response* res_;
	};
}