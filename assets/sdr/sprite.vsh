precision highp float;
attribute vec2 a_pos;
attribute vec2 a_tex;
uniform mat4 u_pr;
uniform mat4 u_mv;
varying vec2 v_tex;
void main()
{
	vec4 pos = vec4(a_pos, 0.0, 1.0);
	vec2 uv = a_tex;
	if (pos.y > 2048.0)
	{
		pos.y = 3328.0 - pos.y;
		float x = uv.x;
		uv.x = uv.y;
		uv.y = x;
	}
	v_tex = uv;
	gl_Position = u_pr * u_mv * pos;
}