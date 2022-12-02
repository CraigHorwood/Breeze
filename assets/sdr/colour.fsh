precision highp float;
uniform vec3 u_col;
void main()
{
	vec4 col = vec4(u_col, 1.0);
	if (col.b < 0.0)
	{
		col.a = -col.b;
		col.b = 1.0;
	}
	gl_FragColor = col;
}