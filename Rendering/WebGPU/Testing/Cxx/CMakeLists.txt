if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(_vtk_test_cxx_wasm_enabled_in_browser ON)
  set(_vtk_test_emscripten_extra_linker_args
    "-sASYNCIFY=1"
    "-sASYNCIFY_STACK_SIZE=81920" #~297 nesting levels
    # O2 or higher make wasm-opt terribly slow.
    # test executables take a minute to link.
    "-O1"
    # we need emscripten to emit code that handles exit code and atexit(s)
    "-sEXIT_RUNTIME=1"
    "-sALLOW_MEMORY_GROWTH=1")
endif ()
# (vtk/vtk#19322): Add baselines for these unit tests
vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  TestActorFaceCullingProperty.cxx,NO_DATA
  TestCellScalarMappedColors.cxx,NO_DATA
  # TestCompositePolyDataMapper.cxx,NO_DATA,NO_VALID # FIXME: Composite PolyDataMapper needs updated to delegator design pattern.
  TestComputeDoublePipelineRenderBuffer.cxx,NO_DATA
  TestComputeFrustumCulling.cxx,NO_DATA,NO_VALID
  TestComputeOcclusionCulling.cxx,NO_DATA,NO_VALID
  TestComputeOcclusionCullingResize.cxx,NO_DATA,NO_VALID
  TestComputeModifyPointColors.cxx,NO_DATA
  TestComputeModifyCellColors.cxx,NO_DATA
  TestComputePass.cxx,NO_DATA,NO_VALID
  TestComputePassChained.cxx,NO_DATA,NO_VALID
  TestComputePassUniform.cxx,NO_DATA,NO_VALID
  TestComputePipelineRelease.cxx,NO_DATA,NO_VALID
  TestComputePointCloudMapper.cxx,NO_DATA
  TestComputePointCloudMapperColors.cxx,NO_DATA
  TestComputePointCloudMapperDepth.cxx,NO_DATA
  TestComputePointCloudMapperGeometry.cxx,NO_DATA
  TestComputePointCloudMapperResize.cxx,NO_DATA
  TestComputeTexture.cxx,NO_DATA,NO_VALID
  TestLineRendering.cxx,NO_DATA
  TestLowPowerRenderWindow.cxx,NO_DATA
  TestPointScalarMappedColors.cxx,NO_DATA
  TestReadPixels.cxx,NO_DATA,NO_VALID
  TestScalarModeToggle.cxx,NO_DATA
  TestSurfacePlusEdges.cxx,NO_DATA
  TestQuad.cxx,NO_DATA
  TestQuadPointRep.cxx,NO_DATA
  TestRenderWindowChangeDeviceLater.cxx,NO_DATA
  TestVertexRendering.cxx,NO_DATA
  TestWireframe.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 0.5
  TestPointRendering_1,TestPointRendering.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 1.0
  TestPointRendering_2,TestPointRendering.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 3.0
  TestPointRendering_3,TestPointRendering.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 7.0
  TestPointRendering_4,TestPointRendering.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 0.5 --line-width 0.5
  TestMixedGeometry_1,TestMixedGeometry.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 1.0 --line-width 1.0
  TestMixedGeometry_2,TestMixedGeometry.cxx,NO_DATA)

vtk_add_test_cxx(vtkRenderingWebGPUCxxTests tests
  --point-size 3.0 --line-width 3.0
  TestMixedGeometry_3,TestMixedGeometry.cxx,NO_DATA)

vtk_test_cxx_executable(vtkRenderingWebGPUCxxTests tests)
