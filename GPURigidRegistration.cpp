#include "itkImage.h"
#include "itkImageFileReader.h"
#include "gpu_rigidregistration.h"
#include "vtkTransform.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        // check existing file
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0];
        std::cerr << " <InputFileName>";
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    // Reading the first file
    typedef itk::Image<float, 3> ImageType;
    using ReaderType = itk::ImageFileReader<ImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    ImageType::Pointer image = reader->GetOutput();

    //Reading the second file 
    ReaderType::Pointer reader2 = ReaderType::New();
    reader->SetFileName(argv[2]);
    reader->Update();

    ImageType::Pointer image2 = reader->GetOutput();

    GPU_RigidRegistration* rigidRegistrator = new GPU_RigidRegistration();

    // Initialize parameters
    rigidRegistrator->SetNumberOfPixels(128000);
    rigidRegistrator->SetOrientationSelectivity(32);
    rigidRegistrator->SetPopulationSize(100);
    rigidRegistrator->SetPercentile(0.8);
    rigidRegistrator->SetUseMask(true);

    // Set image inputs
    rigidRegistrator->SetItkSourceImage(image);
    rigidRegistrator->SetItkTargetImage(image2);

    // Set transform inputs
    vtkTransform * t = vtkTransform::New();
    rigidRegistrator->SetVtkTransform(t);

    rigidRegistrator->runRegistration();

    return EXIT_SUCCESS;
}

