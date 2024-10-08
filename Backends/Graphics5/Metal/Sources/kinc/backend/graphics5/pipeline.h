#pragma once

struct kinc_g5_shader;

typedef struct {
	struct kinc_g5_shader *vertexShader;
	struct kinc_g5_shader *fragmentShader;
	void *_pipeline;
	void *_pipelineDepth;
	void *_reflection;
	void *_depthStencil;
	void *_depthStencilNone;
	// void _set();
} PipelineState5Impl;

typedef struct {
	int a;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} ConstantLocation5Impl;
