#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkEuler3DTransform.h"
#include "itkTransformFileWriter.h"
#include "gpu_rigidregistration.h"
#include "vtkTransform.h"


#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        // check existing file
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0];
        std::cerr << " <MovingImage> <FixedImage>";
        std::cerr << " <InitialTransform> <OutputTransform>";
        std::cerr << std::endl;

        //TODO: add parameter description 

        return EXIT_FAILURE;
    }
    
    using ImageType = itk::Image<float, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;

    // Reading the first file
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[1]);
    
    try
    {
        reader->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer movingImage = reader->GetOutput();

    //Reading the second file 
    ReaderType::Pointer reader2 = ReaderType::New();
    reader->SetFileName(argv[2]);
    
    try
    {
        reader->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer fixedImage = reader->GetOutput();

    //TODO: read initialization transform in a vtkTransform

    GPU_RigidRegistration* rigidRegistrator = new GPU_RigidRegistration();

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

    rigidRegistrator->runRegistration();

    //TODO: write output transform in argv[4]
    using ScalarType = double;
    using ItkRigidTransformType = itk::Euler3DTransform<ScalarType>;
    ItkRigidTransformType::Pointer outputTransform = ItkRigidTransformType::New();
    ItkRigidTransformType::MatrixType matrix;
    ItkRigidTransformType::OffsetType offset;

    for( unsigned int i = 0; i < 3; i++ )
    {
        for( unsigned int j = 0; j < 3; j++ )
        {
            matrix.GetVnlMatrix().set(i, j, transform->GetMatrix()->GetElement(i, j));
        }
        offset[i] = transform->GetMatrix()->GetElement(i, 3);
    }

    outputTransform->SetMatrix(matrix);
    outputTransform->SetOffset(offset);
    
    using TransformWriterType = itk::TransformFileWriterTemplate<ScalarType>;
    TransformWriterType::Pointer writer = TransformWriterType::New();
    writer->SetInput(outputTransform);
    writer->SetFileName(argv[4]);
    try
    {
        writer->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error while saving the transforms:" << error << std::endl;
        return EXIT_FAILURE;
    }

    // temporary testing
    for( size_t i = 0; i < 4; i++ )
    {
        for( size_t j = 0; j < 4; j++ )
        {
            std::cout << transform->GetMatrix()->GetElement(i, j) << "\t";
        }
        std::cout << std::endl;
    }
    

    return EXIT_SUCCESS;
}

