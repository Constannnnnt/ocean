/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "application/ocean/test/cv/testcv/testlibyuv/TestCVLibyuv.h"

#include "ocean/base/Build.h"
#include "ocean/base/CommandArguments.h"
#include "ocean/base/DateTime.h"
#include "ocean/base/Processor.h"
#include "ocean/base/RandomI.h"
#include "ocean/base/String.h"
#include "ocean/base/Timestamp.h"

#include "ocean/platform/System.h"

#include "ocean/system/Memory.h"
#include "ocean/system/OperatingSystem.h"
#include "ocean/system/Process.h"

#include "ocean/test/testcv/testlibyuv/TestLibyuv.h"

using namespace Ocean;

#if defined(_WINDOWS)
	// main function on Windows platforms
	int wmain(int argc, wchar_t* argv[])
#elif defined(__APPLE__) || defined(__linux__)
	// main function on OSX platforms
	int main(int argc, char* argv[])
#else
	#error Missing implementation.
#endif
{
#ifdef _MSC_VER
	// prevent the debugger to abort the application after an assert has been caught
	_set_error_mode(_OUT_TO_MSGBOX);
#endif

#ifdef OCEAN_DEBUG
	constexpr double defaultTestDuration = 0.1;
#else
	constexpr double defaultTestDuration = 2.0;
#endif // OCEAN_DEBUG

	CommandArguments commandArguments;
	commandArguments.registerParameter("resolution", "r", "The default test resolution for all tests, e.g., 1280x720", Value("1280x720"));
	commandArguments.registerParameter("output", "o", "The optional output file for the test log, e.g., log.txt");
	commandArguments.registerParameter("functions", "f", "The optional subset of functions to test, e.g., \"frameshrinker, sse, frameconverter\"");
	commandArguments.registerParameter("duration", "d", "The test duration for each test in seconds, e.g., 1.0", Value(defaultTestDuration));
	commandArguments.registerParameter("help", "h", "Show this help output");

	commandArguments.parse(argv, size_t(argc));

	if (commandArguments.hasValue("help", nullptr, false))
	{
		std::cout << "Ocean Framework test for the CV Libyuv library:" << std::endl << std::endl;
		std::cout << commandArguments.makeSummary() << std::endl;
		return 0;
	}


	double testDuration = 2.0;
	std::string outputFilename;
	std::string functionList;

	Value durationValue;
	if (commandArguments.hasValue("duration", &durationValue, true) && durationValue.isFloat64(true))
	{
		testDuration = durationValue.float64Value(true);
	}

	Value outputValue;
	if (commandArguments.hasValue("output", &outputValue) && outputValue.isString())
	{
		outputFilename = outputValue.stringValue();
	}

	Value functionsValue;
	if (commandArguments.hasValue("functions", &functionsValue) && functionsValue.isString())
	{
		functionList = functionsValue.stringValue();
	}

	Messenger::MessageOutput messageOutput = Messenger::OUTPUT_STANDARD;

	if (!outputFilename.empty() && outputFilename != "STANDARD")
	{
		Messenger::get().setFileOutput(outputFilename);
		messageOutput = Messenger::MessageOutput(messageOutput | Messenger::OUTPUT_FILE);
	}

	Messenger::get().setOutputType(messageOutput);

	const Timestamp startTimestamp(true);

	Log::info() << "Test for the Libyuv library:";
	Log::info() << " ";
	Log::info() << "Platform: " << Build::buildString();
	Log::info() << " ";
	Log::info() << "Start: " << DateTime::stringDate() << ", " << DateTime::stringTime() << " UTC";
	Log::info() << " ";

	Log::info() << "Function list: " << (functionList.empty() ? "All functions" : functionList);
	Log::info() << "Duration for each test: " << String::toAString(testDuration, 1u) << "s";
	Log::info() << " ";

	RandomI::initialize();
	System::Process::setPriority(System::Process::PRIORITY_ABOVE_NORMAL);

	Log::info() << "Random generator initialized";
	Log::info() << "Process priority set to above normal";
	Log::info() << " ";

	Worker worker;

	Log::info() << "Operating System: " << String::toAString(System::OperatingSystem::name());
	Log::info() << "Processor: " << Processor::brand();
	Log::info() << "Used worker threads: " << worker.threads();
	Log::info() << " ";

	const unsigned long long startVirtualMemory = System::Memory::processVirtualMemory();

	Log::info() << "Currently used memory: " << String::insertCharacter(String::toAString(startVirtualMemory >> 10), ',', 3, false) << "KB";
	Log::info() << " ";

	// True, to skip the validation and to apply just the benchmarking
	const bool skipValidation = false;

	int resultValue = 1;

	try
	{
		Test::TestCV::TestLibyuv::testCVLibyuv(testDuration, skipValidation, functionList);
	}
	catch (const std::exception& exception)
	{
		ocean_assert(false && "Unhandled exception!");
		Log::info() << "Unhandled exception: " << exception.what();
	}
	catch (...)
	{
		ocean_assert(false && "Unhandled exception!");
		Log::info() << "Unhandled exception!";
	}

	const unsigned long long stopVirtualMemory = System::Memory::processVirtualMemory();

	Log::info() << " ";
	Log::info() << "Currently used memory: " << String::insertCharacter(String::toAString(stopVirtualMemory >> 10), ',', 3, false) << "KB (+ " << String::insertCharacter(String::toAString((stopVirtualMemory - startVirtualMemory) >> 10), ',', 3, false) << "KB)";
	Log::info() << " ";

	const Timestamp endTimestamp(true);

	Log::info() << "Time elapsed: " << DateTime::seconds2string(double(endTimestamp - startTimestamp), true);
	Log::info() << "End: " << DateTime::stringDate() << ", " << DateTime::stringTime() << " UTC";
	Log::info() << " ";

	if (Messenger::get().outputType() == Messenger::OUTPUT_STANDARD)
	{
		std::cout << "Press a key to exit.";
		getchar();
	}

	return resultValue;
}
