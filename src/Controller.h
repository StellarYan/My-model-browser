#pragma once


struct Controller
{
	float left;
	float right;
	float forward;
	float backward;

	float lastX;
	float lastY;
	float deltaX;
	float deltaY;
	float yaw = 0;
	float pitch = 0;


	bool isMouseMoving = false;
	bool isCastRay = false;
	bool isHoldingObject = false;
	bool isEnablePostEffect = false;
	bool isCloseWindow = false;
	float holdingDistance = 20;
	float scaleParameter = 1.0;

	GLShaderObject currentShader=1;
	GLShaderObject PostEffectShader=0;

	float sensitivity = 1.0f;
	float speed = 1.0f;
}controller;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) controller.isEnablePostEffect = !controller.isEnablePostEffect;
	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) controller.forward = 1.0f;
	else controller.forward = 0.0f;
	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) controller.backward = 1.0f;
	else controller.backward = 0.0f;
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) controller.left = 1.0f;
	else controller.left = 0.0f;
	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) controller.right = 1.0f;
	else controller.right = 0.0f;
	if (GLFW_KEY_0 <= key  && key <= GLFW_KEY_9) controller.currentShader = key - GLFW_KEY_0;
	if (key == GLFW_KEY_X && action == GLFW_PRESS)  controller.isCastRay = true;
	else controller.isCastRay = false;
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS) controller.speed *= 2;
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS) controller.speed /= 2;


	if (controller.isHoldingObject)
	{
		if (key == GLFW_KEY_I && action == GLFW_PRESS )
		{
			controller.scaleParameter += 0.1;
		}
		else if (key == GLFW_KEY_K && action == GLFW_PRESS )
		{
			controller.scaleParameter -= 0.1;
		}
	}
	else
	{
		controller.scaleParameter = 1.0f;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	controller.isMouseMoving = true;
	controller.deltaX = xpos - controller.lastX;
	controller.deltaY = ypos - controller.lastY;
	controller.lastX = xpos;
	controller.lastY = ypos;
	controller.yaw += controller.deltaX*controller.sensitivity;
	if (glm::abs(controller.pitch + controller.deltaY*controller.sensitivity)<90) controller.pitch += controller.deltaY*controller.sensitivity;

}

void mouseButton_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) controller.isCastRay = true;
	else controller.isCastRay = false;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) controller.isHoldingObject = true;
	else controller.isHoldingObject = false;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (controller.isHoldingObject)
	{
		double dis = controller.holdingDistance + yoffset;
		controller.holdingDistance = (dis > 0 && dis < 150) ? dis : controller.holdingDistance;
	}
}
