/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "application/ocean/demo/tracking/homographytracker/android/GLMainView.h"

#include "ocean/base/String.h"

#include "ocean/media/FrameMedium.h"
#include "ocean/media/Manager.h"

#include "ocean/platform/android/Utilities.h"

using namespace Ocean;

const bool GLMainView::instanceRegistered_ = GLMainView::registerInstanceFunction(GLMainView::createInstance);

GLMainView::GLMainView() :
	recentTouchPosition_(Numeric::minValue(), Numeric::maxValue())
{
	pixelImage_ = Media::Manager::get().newMedium("PixelImageForRenderer", Media::Medium::PIXEL_IMAGE);
}

GLMainView::~GLMainView()
{
	stopThread();
	release();
}

void GLMainView::initializeHomographyTracker(const std::string& inputMedium, const std::string& resolution)
{
	std::vector<std::wstring> commandLines;

	commandLines.push_back(String::toWString(inputMedium));
	commandLines.push_back(String::toWString(resolution));

	homographyTrackerWrapper_ = HomographyTrackerWrapper(commandLines);

	if (homographyTrackerWrapper_.frameMedium())
	{
		pixelImage_->setDevice_T_camera(homographyTrackerWrapper_.frameMedium()->device_T_camera());
	}

	if (!setBackgroundMedium(pixelImage_, true))
	{
		Log::error() << "Failed to set the background medium";
	}

	startThread();
}

void GLMainView::threadRun()
{
	double resultingTrackerPerformance;

	while (shouldThreadStop() == false)
	{
		// we check whether the platform independent tracker has some new image to process

		const Vector2 localRecentTouchPosition = recentTouchPosition_;

		Frame resultingTrackerFrame;
		if (homographyTrackerWrapper_.trackNewFrame(resultingTrackerFrame, resultingTrackerPerformance, localRecentTouchPosition) && resultingTrackerFrame.isValid())
		{
			// we received an augmented frame from the tracker
			// so we forward the result to the render by updating the visual content of the pixel image

			// **NOTE** copying the resulting RGB frame and forwarding the frame to the renderer costs some performance
			// however, this demo application focuses on the usage of platform independent code and not on performance
			// @see ocean_app_shark for a high performance implementation of an Augmented Realty application (even more powerful)

			pixelImage_->setPixelImage(std::move(resultingTrackerFrame));

			Log::info() << resultingTrackerPerformance * 1000.0 << "ms";

			recentTouchPosition_ = Vector2(Numeric::minValue(), Numeric::minValue());
		}
		else
		{
			Thread::sleep(1u);
		}
	}

}

void GLMainView::onTouchDown(const float x, const float y)
{
	Scalar xFrame, yFrame;

	if (screen2frame(x, y, xFrame, yFrame))
	{
		recentTouchPosition_ = Vector2(xFrame, yFrame);
	}
}

jboolean Java_com_meta_ocean_app_demo_tracking_homographytracker_android_HomographyTrackerActivity_initializeHomographyTracker(JNIEnv* env, jobject javaThis, jstring inputMedium, jstring resolution)
{
	const std::string inputMediumValue(Platform::Android::Utilities::toAString(env, inputMedium));
	const std::string resolutionValue(Platform::Android::Utilities::toAString(env, resolution));

	GLMainView::get<GLMainView>().initializeHomographyTracker(inputMediumValue, resolutionValue);

	return true;
}
