

#define DISABLE_LOGGER

#include <cinatra/cinatra.hpp>
#include <fstream>

struct CheckLoginAspect
{
	void before(const Request& req, Response& res)
	{
		//std::cout << req.url() << endl;
	}

	void after(const Request& req, Response& res)
	{

	}
};

struct MyStruct
{
	void hello(const cinatra::Request& req, cinatra::Response& res)
	{
		res.end("Hello noname!");
	}
};

int main()
{
	cinatra::Cinatra<CheckLoginAspect> app;
	app.route("/", [](const cinatra::Request& req, cinatra::Response& res)
	{
		res.end("Hello Cinatra");
	});

	// 访问/login.html进行登录.
	app.route("/test_post", [](const cinatra::Request& req, cinatra::Response& res)
	{
		if (req.method() != Request::method_t::POST)
		{
			res.set_status_code(404);
			res.end("404 Not found");
			return;
		}

		auto body = cinatra::body_parser(req.body());
		res.end("Hello " + body.get_val("uid") + "! Your password is " + body.get_val("pwd") + "...hahahahaha...");
	});


	MyStruct t;
	// 访问/hello
	app.route("/hello", &MyStruct::hello, &t);
	// 访问类似于/hello/jone/10/xxx
	// joen、10和xxx会分别作为a、b和c三个参数传入handler
	app.route("/hello/:name/:age/:test", [](const cinatra::Request& req, cinatra::Response& res, const std::string& a, int b, double c)
	{
		res.end("Name: " + a + " Age: " + lexical_cast<std::string>(b)+"Test: " + lexical_cast<std::string>(c));
	});
	// 
	app.route("/hello/:name/:age", [](const cinatra::Request& req, cinatra::Response& res, const std::string& a, int b)
	{
		res.end("Name: " + a + " Age: " + lexical_cast<std::string>(b));
	});

	// example: /test_query?a=asdf&b=sdfg
	app.route("/test_query", [](const cinatra::Request& req, cinatra::Response& res)
	{
		res.write("<html><head><title>test query</title ></head><body>");
		res.write("Total " + lexical_cast<std::string>(req.query().size()) + "queries<br />");
		for (auto it : req.query())
		{
			res.write(it.first + ": " + it.second + "<br />");
		}

		res.end("</body></html>");
	});

	//设置cookie
	app.route("/set_cookies", [](const cinatra::Request& req, cinatra::Response& res)
	{
		res.cookies().new_cookie() // 会话cookie
			.add("foo", "bar")
			.new_cookie().max_age(24 * 60 * 60) //这个cookie会保存一天的时间
			.add("longtime", "test");
		res.write("<html><head><title>Set cookies</title ></head><body>");
		res.write("Please visit <a href='/show_cookies'>show cookies page</a>");
		res.end("</body></html>");
	});
	//列出所有的cookie
	app.route("/show_cookies", [](const cinatra::Request& req, cinatra::Response& res)
	{
		res.write("<html><head><title>Show cookies</title ></head><body>");
		res.write("Total " + lexical_cast<std::string>(req.cookie().size()) + "cookies<br />");
		for (auto it : req.cookie())
		{
			res.write(it.first + ": " + it.second + "<br />");
		}

		res.end("</body></html>");
	});


	app.error_handler(
		[](int code, const std::string&, const cinatra::Request&, cinatra::Response& res)
	{
		if (code != 404)
		{
			return false;
		}

		res.set_status_code(404);
		res.write(
			R"(<html>
			<head>
				<meta charset="UTF-8">
				<title>404</title>
			</head>
			<body>
			<img src="/img/404.jpg" width="100%" height="100%" />
			</body>
			</html>)"
		);

		return true;
	});

	app.public_dir("./public").threads(std::thread::hardware_concurrency()).listen("0.0.0.0", "http").run();

	return 0;
}
