precision highp float;
uniform sampler2D u_tex;
uniform vec4 u_col;
varying vec2 v_tex;
void main()
{
	if (u_col.a != 0.0) gl_FragColor = texture2D(u_tex, v_tex) * u_col;
	else gl_FragColor = texture2D(u_tex, v_tex);
}