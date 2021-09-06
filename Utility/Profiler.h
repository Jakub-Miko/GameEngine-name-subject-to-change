#pragma once
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iomanip>

// Profiler class - singleton maintaining session and file writes to the .json file used with chrome://tracing

class Profiler {
private:
	struct Session {
		const char* name;
		const char* filepath;
		std::ofstream output_stream;
		bool first = true;
	};

	std::mutex global_mutex;
	bool is_active_session = false;
	Session current_session;
public:

	struct TraceResult {
		TraceResult(std::chrono::nanoseconds start, std::chrono::nanoseconds duration, const char* name, std::thread::id thread_id)
			: start(start), duration(duration), name(name), thread_id(thread_id) { };
		
		std::chrono::nanoseconds start;
		std::chrono::nanoseconds duration;
		const char* name;
		std::thread::id thread_id;
	};

public:
	Profiler(const Profiler& other) = delete;
	Profiler& operator=(const Profiler& other) = delete;

	static Profiler* Get() {
		static Profiler ProfilerInstance;
		return &ProfilerInstance;
	}

	void StartSession(const char* name, const char* filepath) {
		std::lock_guard<std::mutex> guard(global_mutex);
		if (is_active_session) {
			//__debugbreak();
		}
		current_session.output_stream.open(filepath);
		if (!current_session.output_stream.is_open()) {
			//__debugbreak();
		}

		current_session.name = name;
		current_session.filepath = filepath;
		is_active_session = true;
		current_session.first = true;
		WriteHeader();
	}

	void StopSession() {
		std::lock_guard<std::mutex> guard(global_mutex);
		if (!is_active_session) {
			//__debugbreak();
		}
		WriteFooter();
		current_session.filepath = "";
		current_session.name = "";
		current_session.output_stream.close();
		is_active_session = false;
	}

	void TraceEvent(const TraceResult& results) {
		if (!is_active_session) {
			return;
		}
		
		std::stringstream ss;
		
		if (!current_session.first) {
			ss << ","; 
		}
		else {
			current_session.first = false;
		}
		ss << std::fixed << std::setprecision(3);
		ss << "{\"name\": " << "\"" << results.name << "\",";
		ss << "\"cat\": " << "\"Default\",";
		ss << "\"ph\": " << "\"X\",";
		ss << "\"ts\": " << (long long)results.start.count() * 0.001 << ",";
		ss << "\"dur\": " << (long long)results.duration.count() * 0.001 << ",";
		ss << "\"pid\": " << "1,";
		ss << "\"tid\": " << results.thread_id << "";
		ss << "}";
		
		{
			std::lock_guard<std::mutex> guard(global_mutex);
			current_session.output_stream << ss.str();
			current_session.output_stream.flush();
		}
	}


private:

	void WriteHeader() {
		current_session.output_stream << "{ \"traceEvents\": [";
		current_session.output_stream.flush();
	}

	void WriteFooter() {
		current_session.output_stream << "], \"displayTimeUnit\": \"ms\" }";
		current_session.output_stream.flush();
	}

	Profiler() = default;
};

// ProfilerTimer class - Timers Placed in code capture their own lifetimes and submit them to the Profiler class at destruction.

class ProfilerTimer {
private:
	std::chrono::steady_clock::time_point start;
	const char* name;
	bool running;
public:
	ProfilerTimer(const char* name) : name(name), running(true) {
		start = std::chrono::steady_clock::now();
	}

	~ProfilerTimer() {
		if (running) Stop();
	}

	void Stop() {
		running = false;
		auto end = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch();
		auto startpoint = std::chrono::time_point_cast<std::chrono::nanoseconds>(start).time_since_epoch();
		auto nano = (end - startpoint);

		std::thread::id thread_id = std::this_thread::get_id();
		Profiler::TraceResult trace_result(startpoint, nano, name, thread_id);
		Profiler::Get()->TraceEvent(trace_result);
	}

};

// Macros for easy placement of timers and maintaining sessions, as well as for striping the code from release builds.

#define ENABLE_PROFILING
#ifdef ENABLE_PROFILING
	
		#ifndef COMBINE
		#define COMBINE2(a, b) a ## b
		#define COMBINE(a, b) COMBINE2(a, b)
		#endif
	
	#define BEGIN_PROFILING(name, path) Profiler::Get()->StartSession(name,path)
	#define END_PROFILING() Profiler::Get()->StopSession()
	#define PROFILE(name) ProfilerTimer COMBINE(timer,__LINE__)(name)
	#define PROFILE_FUNCTION() PROFILE(__FUNCSIG__)
	#define PROFILE_USAGE(line) line
	
#else 
	
	#define BEGIN_PROFILING(name, path)
	#define END_PROFILING()
	#define PROFILE(name) 
	#define PROFILE_FUNCTION()
	#define PROFILE_USAGE(line)
	
#endif