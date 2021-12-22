#include "test/rest-api/MockDataExchange.hpp"

using namespace testing;

std::unique_ptr<NiceMock<MockParticleWrapper>> createMockParticleWrapper(
    const std::string& name,
    RestApi::Interval<float> size,
    RestApi::Interval<float> rate,
    RestApi::Interval<float> lifetime,
    int fadeoutTime,
    const std::string& shape,
    RestApi::Interval<uint32_t> color,
    FloatArray3 boxSize,
    float sphereRadius,
    int angleSpread,
    FloatArray3 velocity,
    int emissionDecayRate,
    FloatArray2 scaleAffactor,
    bool flips,
    bool vertical,
    bool randomY)
{
    auto particles = std::make_unique<NiceMock<MockParticleWrapper>>();
    ON_CALL(*particles, getName()).WillByDefault(Return(name));
    ON_CALL(*particles, getSize()).WillByDefault(Return(size));
    ON_CALL(*particles, getRate()).WillByDefault(Return(rate));
    ON_CALL(*particles, getLifetime()).WillByDefault(Return(lifetime));
    ON_CALL(*particles, getFadeoutTime()).WillByDefault(Return(fadeoutTime));
    ON_CALL(*particles, getShape()).WillByDefault(Return(shape));
    ON_CALL(*particles, getMaterial()).WillByDefault(Return(std::nullopt));
    ON_CALL(*particles, getColor()).WillByDefault(Return(color));
    ON_CALL(*particles, getBoxSize()).WillByDefault(Return(boxSize));
    ON_CALL(*particles, getSphereRadius()).WillByDefault(Return(sphereRadius));
    ON_CALL(*particles, getAngleSpread()).WillByDefault(Return(angleSpread));
    ON_CALL(*particles, getVelocity()).WillByDefault(Return(velocity));
    ON_CALL(*particles, getEmissionDecayRate()).WillByDefault(Return(emissionDecayRate));
    ON_CALL(*particles, getScaleAffactor()).WillByDefault(Return(scaleAffactor));
    ON_CALL(*particles, hasFlips()).WillByDefault(Return(flips));
    ON_CALL(*particles, isVertical()).WillByDefault(Return(vertical));
    ON_CALL(*particles, hasRandomInitialY()).WillByDefault(Return(randomY));
    return particles;
}
