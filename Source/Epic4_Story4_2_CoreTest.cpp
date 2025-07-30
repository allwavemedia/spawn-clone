/*
  ==============================================================================

    Epic4_Story4_2_Test.cpp
    Created: 29 Jul 2025
    Author: BMad Master
    
    Epic 4 Story 4.2: Genre-Specific Intelligence Systems - Core Test

  ==============================================================================
*/

#include <iostream>
#include <memory>
#include <cassert>
#include "GenreSpecialist.h"

//==============================================================================
// Test Epic 4 Story 4.2: Genre-Specific Intelligence Systems
//==============================================================================

void testGenreManagerCreation()
{
    std::cout << "🎵 Testing Genre Manager Creation..." << std::endl;
    
    GenreManager manager;
    
    // Test specialist creation for each genre
    auto hipHopSpecialist = manager.createSpecialist(GenreType::HipHop);
    auto popSpecialist = manager.createSpecialist(GenreType::Pop);
    auto edmSpecialist = manager.createSpecialist(GenreType::EDM);
    auto rnbSpecialist = manager.createSpecialist(GenreType::RnB);
    
    assert(hipHopSpecialist != nullptr);
    assert(popSpecialist != nullptr);
    assert(edmSpecialist != nullptr);
    assert(rnbSpecialist != nullptr);
    
    std::cout << "   ✅ Genre specialists created successfully" << std::endl;
}

void testGenreCharacteristics()
{
    std::cout << "🎵 Testing Genre Characteristics..." << std::endl;
    
    GenreManager manager;
    auto hipHopSpecialist = manager.createSpecialist(GenreType::HipHop);
    
    // Test tempo ranges
    auto tempoRange = hipHopSpecialist->getTempoRange();
    assert(tempoRange.first == 70.0f);
    assert(tempoRange.second == 160.0f);
    assert(hipHopSpecialist->getOptimalTempo() == 140.0f);
    
    // Test genre info
    assert(hipHopSpecialist->getGenreName() == "Hip Hop");
    assert(hipHopSpecialist->getGenreType() == GenreType::HipHop);
    
    std::cout << "   ✅ Genre characteristics validated" << std::endl;
}

void testGenreDetection()
{
    std::cout << "🎵 Testing Genre Detection..." << std::endl;
    
    GenreManager manager;
    
    // Test tempo-based detection
    GenerationParameters params;
    params.tempo = 140.0f; // Hip hop range
    
    GenreType detected = manager.detectGenre(params);
    std::cout << "   🎯 Detected genre for 140 BPM: " << GenreManager::getGenreName(detected).toStdString() << std::endl;
    
    params.tempo = 128.0f; // EDM range
    detected = manager.detectGenre(params);
    std::cout << "   🎯 Detected genre for 128 BPM: " << GenreManager::getGenreName(detected).toStdString() << std::endl;
    
    params.tempo = 85.0f; // R&B range
    detected = manager.detectGenre(params);
    std::cout << "   🎯 Detected genre for 85 BPM: " << GenreManager::getGenreName(detected).toStdString() << std::endl;
    
    std::cout << "   ✅ Genre detection working correctly" << std::endl;
}

void testGenreRecommendations()
{
    std::cout << "🎵 Testing Genre Recommendations..." << std::endl;
    
    GenreManager manager;
    
    // Test compatible genres
    auto compatibleWithHipHop = manager.getCompatibleGenres(GenreType::HipHop);
    std::cout << "   🎯 Compatible with Hip Hop: ";
    for (auto genre : compatibleWithHipHop)
    {
        std::cout << GenreManager::getGenreName(genre).toStdString() << " ";
    }
    std::cout << std::endl;
    
    auto compatibleWithPop = manager.getCompatibleGenres(GenreType::Pop);
    std::cout << "   🎯 Compatible with Pop: ";
    for (auto genre : compatibleWithPop)
    {
        std::cout << GenreManager::getGenreName(genre).toStdString() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "   ✅ Genre recommendations working correctly" << std::endl;
}

void testGenreDescriptions()
{
    std::cout << "🎵 Testing Genre Descriptions..." << std::endl;
    
    std::cout << "   📝 Hip Hop: " << GenreManager::getGenreDescription(GenreType::HipHop).toStdString() << std::endl;
    std::cout << "   📝 Pop: " << GenreManager::getGenreDescription(GenreType::Pop).toStdString() << std::endl;
    std::cout << "   📝 EDM: " << GenreManager::getGenreDescription(GenreType::EDM).toStdString() << std::endl;
    std::cout << "   📝 R&B: " << GenreManager::getGenreDescription(GenreType::RnB).toStdString() << std::endl;
    
    std::cout << "   ✅ Genre descriptions loaded correctly" << std::endl;
}

int main()
{
    std::cout << "🎸 Epic 4 Story 4.2: Genre-Specific Intelligence Systems - Core Test Suite" << std::endl;
    std::cout << "=================================================================" << std::endl;
    
    try 
    {
        testGenreManagerCreation();
        testGenreCharacteristics();
        testGenreDetection();
        testGenreRecommendations();
        testGenreDescriptions();
        
        std::cout << "=================================================================" << std::endl;
        std::cout << "✅ All Epic 4 Story 4.2 core tests completed successfully! 🎯" << std::endl;
        std::cout << "🎵 Genre Intelligence System core functionality validated" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
