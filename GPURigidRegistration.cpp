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
    typedef itk::Image<float, 3> IbisItkFloat3ImageType;
    using ReaderType = itk::ImageFileReader<IbisItkFloat3ImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    IbisItkFloat3ImageType::Pointer image = reader->GetOutput();

  //Reading the second file 
    using ReaderType = itk::ImageFileReader<IbisItkFloat3ImageType>;
    ReaderType::Pointer reader2 = ReaderType::New();
    reader->SetFileName(argv[2]);
    reader->Update();

    IbisItkFloat3ImageType::Pointer image2 = reader->GetOutput();

    return EXIT_SUCCESS;

    GPU_RigidRegistration* rigidRegistrator = new GPU_RigidRegistration();

    // Initialize parameters
    /*m_rigidRegistrator->SetNumberOfPixels(ui->numebrOfPixelsDial->value());
    m_rigidRegistrator->SetOrientationSelectivity(ui->selectivityDial->value());
    m_rigidRegistrator->SetPopulationSize(ui->populationSizeDial->value());
    m_rigidRegistrator->SetPercentile(ui->percentileComboBox->itemData(ui->percentileComboBox->currentIndex()).toDouble());
    m_rigidRegistrator->SetUseMask(ui->computeMaskCheckBox->isChecked());
    m_rigidRegistrator->SetDebug(debug, &debugStringStream);*/

    // Set image inputs
    rigidRegistrator->SetItkSourceImage(image);
    rigidRegistrator->SetItkTargetImage(image2);

    // Set transform inputs
    vtkTransform * t = vtkTransform::New();
    rigidRegistrator->SetVtkTransform(t);

    rigidRegistrator->runRegistration();

    delete rigidRegistrator;
    t->Delete();
}

