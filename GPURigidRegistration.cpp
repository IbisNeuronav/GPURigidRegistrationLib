#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkEuler3DTransform.h"
#include "itkTransformFileWriter.h"
#include "gpu_rigidregistration.h"
#include "vtkTransform.h"
#include "itkTransformFactory.h"


#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "ARGC: " << argc << std::endl;
    if (argc < 3)
    {
        // check existing file
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0];
        std::cerr << " <MovingImage> <FixedImage>";
        std::cerr << " [<OutputTransform>] [<InitialTransform>]";
        std::cerr << std::endl;

        //TODO: add parameter description 

        return EXIT_FAILURE;
    }
    
    using ImageType = itk::Image<float, 3>;
    using MovingImageReaderType = itk::ImageFileReader<ImageType>;
    using FixedImageReaderType = itk::ImageFileReader<ImageType>;

    // Reading the first file
    MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
    movingReader->SetFileName(argv[1]);
    std::cout << "Reading Moving Image... " << argv[1] << std::endl;
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

    //Reading the second file 
    FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
    fixedReader->SetFileName(argv[2]);
    std::cout << "Reading Fixed Image... " << argv[2] << std::endl;
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

    //TODO: read initialization transform in a vtkTransform
    std::string initialTransformFile = "";
    if( argc > 4 )
    {
        initialTransformFile = argv[4];
    }

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

    // Write output transform
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
    
    std::string outputTransformFile = "outputTransform.h5";
    if( argc > 3 )
        outputTransformFile = argv[3];

    std::cout << "Write transform in " << outputTransformFile << std::endl;
    using TransformWriterType = itk::TransformFileWriterTemplate<ScalarType>;
    TransformWriterType::Pointer writer = TransformWriterType::New();
    writer->SetInput(outputTransform->GetInverseTransform());
    writer->SetFileName(outputTransformFile.c_str());
    try
    {
        writer->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error while saving the transforms:" << error << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

