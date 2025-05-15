#pragma once

class Settings {
public:
	class Graphics {
	public:
		int viewportWidth = 1280;
		int viewportHeight = 720;
		float clipPlaneNear = 0.1f;
		float clipPlaneFar = 1000.0f;
		int maxFrameRate = 0;
	};
	Graphics graphics;
	class Window {
	public:
		const char* windowTitle = "DefaultTitle";
		int windowWidth = 1280;
		int windowHeight = 720;
	};
	Window window;
	class Controls {
	public:
		float cameraSensitivity = 2.0f;
		float cameraSpeed = 0.05f;
	};
	Controls controls;


	Settings() {
		graphics = Graphics();
		window = Window();
		controls = Controls();
	}
};