
[RootSignature("RootFlags(0)")]
float4 main(float4 Position : SV_Position) : SV_TARGET
{
	return float4(Position.x / 1280.0f, Position.y / 720.0f, 0.0f, 1.0f);
}

