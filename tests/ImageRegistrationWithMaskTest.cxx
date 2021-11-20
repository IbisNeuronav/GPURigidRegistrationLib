#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageMaskSpatialObject.h"
#include "../gpu_rigidregistration.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

#include "../utils/vtkXFMReader.h"
#include "../utils/vtkXFMWriter.h"

#include <iostream>

int main(int argc, char * argv[])
{
    if( argc < 2 )
    {
        return EXIT_FAILURE;
    }

    std::string dataDir = argv[1];
    std::string movingImageFileName;
    std::string fixedImageFileName;
    std::string movingMaskFileName;

    movingImageFileName = dataDir + "/mr.mnc";
    fixedImageFileName = dataDir + "/us.mnc";
    movingMaskFileName = dataDir + "/mr_mask.mnc";

    const unsigned int Dimension = 3;
    using PixelType = float;
    using ImageType = itk::Image<PixelType, Dimension>;
    using MovingImageReaderType = itk::ImageFileReader<ImageType>;
    using FixedImageReaderType = itk::ImageFileReader<ImageType>;
    using ImageMaskType = itk::ImageMaskSpatialObject< 3 >;

    ImageMaskType::Pointer movingMask = nullptr;

    // Reading moving image file
    MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
    movingReader->SetFileName(movingImageFileName.c_str());
    try
    {
        movingReader->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer movingImage = movingReader->GetOutput();

    //Reading fixed image file 
    FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
    fixedReader->SetFileName(fixedImageFileName.c_str());
    try
    {
        fixedReader->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer fixedImage = fixedReader->GetOutput();

    // Reading moving mask file
    using MaskReaderType = itk::ImageFileReader<ImageMaskType::ImageType>;
    MaskReaderType::Pointer movingMaskReader = MaskReaderType::New();
    movingMaskReader->SetFileName(movingMaskFileName.c_str());

    try
    {
        movingMaskReader->Update();
        movingMask = ImageMaskType::New();
        movingMask->SetImage(movingMaskReader->GetOutput());
        movingMask->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    GPU_RigidRegistration * rigidRegistrator = new GPU_RigidRegistration();

    // Initialize parameters
    rigidRegistrator->SetNumberOfPixels(128000);
    rigidRegistrator->SetOrientationSelectivity(32);
    rigidRegistrator->SetPopulationSize(100);
    rigidRegistrator->SetPercentile(0.8);
    rigidRegistrator->SetUseMask(true);

    // Set image inputs
    rigidRegistrator->SetItkSourceImage(movingImage);
    rigidRegistrator->SetItkTargetImage(fixedImage);

    // Set transform inputs
    vtkTransform * transform = vtkTransform::New();
    rigidRegistrator->SetVtkTransform(transform);

    if( movingMask )
    {
        rigidRegistrator->SetSourceMask(movingMask);
    }
    else
    {
        return EXIT_FAILURE;
    }

    rigidRegistrator->runRegistration();

    return EXIT_SUCCESS;
}

