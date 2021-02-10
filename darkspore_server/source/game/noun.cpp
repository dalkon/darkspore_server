
// Include
#include "noun.h"
#include "config.h"

#include "utils/functions.h"
#include "utils/log.h"

#include <filesystem>

// Game
namespace Game {
	// BoundingBox
	BoundingBox::BoundingBox() {
		mCenter = glm::vec3(0);
		mExtent = glm::vec3(0);
	}

	BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) {
		mCenter = (max + min) * glm::vec3(0.5f);
		mExtent = (max - min) * glm::vec3(0.5f);
	}

	bool BoundingBox::Contains(const glm::vec3& point) const {
		auto distance = glm::abs(mCenter - point);
		return glm::all(glm::lessThan(distance, mExtent));
	}

	bool BoundingBox::Contains(const BoundingBox& boundingBox) const {
		auto distance = glm::abs(mCenter - boundingBox.mCenter);
		auto distanceRequired = mExtent + boundingBox.mExtent;
		return glm::all(glm::lessThan(distance, distanceRequired));
	}

	bool BoundingBox::Intersects(const BoundingBox& boundingBox) const {
		auto distance = glm::abs(mCenter - boundingBox.mCenter);
		auto distanceRequired = mExtent + boundingBox.mExtent;
		return glm::all(glm::lessThanEqual(distance, distanceRequired));
	}

	bool BoundingBox::Touches(const BoundingBox& boundingBox) const {
		auto distance = glm::abs(mCenter - boundingBox.mCenter);
		auto distanceRequired = mExtent + boundingBox.mExtent;

		auto results = glm::equal(distance, distanceRequired);
		return (results.x + results.y + results.z) >= 2;
	}

	void BoundingBox::Transform(const glm::mat4& matrix) {
		glm::vec3 min = GetMin();
		glm::vec3 max = GetMax();

		glm::vec3 xa = glm::vec3(matrix[0] * min.x);
		glm::vec3 xb = glm::vec3(matrix[0] * max.x);
		glm::vec3 xMin = glm::min(xa, xb);
		glm::vec3 xMax = glm::max(xa, xb);

		glm::vec3 ya = glm::vec3(matrix[1] * min.y);
		glm::vec3 yb = glm::vec3(matrix[1] * max.y);
		glm::vec3 yMin = glm::min(ya, yb);
		glm::vec3 yMax = glm::max(ya, yb);

		glm::vec3 za = glm::vec3(matrix[2] * min.z);
		glm::vec3 zb = glm::vec3(matrix[2] * max.z);
		glm::vec3 zMin = glm::min(za, zb);
		glm::vec3 zMax = glm::max(za, zb);

		glm::vec3 newMin = xMin + yMin + zMin + glm::vec3(matrix[3]);
		glm::vec3 newMax = xMax + yMax + zMax + glm::vec3(matrix[3]);

		*this = BoundingBox(newMin, newMax);
	}

	glm::vec3 BoundingBox::GetMin() const {
		return mCenter - mExtent;
	}

	glm::vec3 BoundingBox::GetMax() const {
		return mCenter + mExtent;
	}

	glm::vec3 BoundingBox::GetSize() const {
		return mExtent * 2.0f;
	}

	// Noun
	void Noun::Read(pugi::xml_node node) {
		auto get_vec3 = [](pugi::xml_node node) {
			glm::vec3 data;
			data.x = utils::xml_get_text_node<float>(node, "x");
			data.y = utils::xml_get_text_node<float>(node, "y");
			data.z = utils::xml_get_text_node<float>(node, "z");
			return data;
		};

		if (node.child("nounType")) {
			// Directly parsed from .Noun files
			mType = utils::xml_get_text_node<NounType>(node, "nounType");
			if (auto bboxNode = node.child("bbox")) {
				mBoundingBox = BoundingBox(
					get_vec3(bboxNode.child("min")),
					get_vec3(bboxNode.child("max"))
				);
			}

			mNpcClassData = utils::xml_get_text_node(node, "npcClassData");
			mPlayerClassData = utils::xml_get_text_node(node, "playerClassData");
			mCharacterAnimationData = utils::xml_get_text_node(node, "characterAnimationData");

			mAssetId = utils::xml_get_text_node<uint64_t>(node, "assetId");
			for (const auto& assetNode : node.child("eliteAssetIds").children()) {
				if (assetNode.name() == "entry") {
					std::string_view assetNodeText = assetNode.text().get();
					mEliteAssetIds.push_back(utils::to_number<uint64_t>(assetNodeText));
				}
			}

			mIsFlora = utils::xml_get_text_node<bool>(node, "isFlora");
			mIsMineral = utils::xml_get_text_node<bool>(node, "isMineral");
			mIsCreature = utils::xml_get_text_node<bool>(node, "isCreature");
			mIsPlayer = utils::xml_get_text_node<bool>(node, "isPlayer");
			mIsSpawned = utils::xml_get_text_node<bool>(node, "isSpawned");
		} else {
			// Shortened versions
		}
	}

	const BoundingBox& Noun::GetBoundingBox() const {
		return mBoundingBox;
	}

	const std::string& Noun::GetName() const {
		return mName;
	}

	uint64_t Noun::GetAssetId() const {
		return mAssetId;
	}

	uint32_t Noun::GetId() const {
		return mId;
	}

	NounType Noun::GetType() const {
		return mType;
	}

	bool Noun::IsCreature() const {
		return mIsCreature;
	}

	bool Noun::IsPlayer() const {
		return mIsPlayer;
	}

	// NounDatabase
	NounDatabase& NounDatabase::Instance() {
		static NounDatabase instance;
		if (!instance.mLoaded) {
			instance.Load();
		}
		return instance;
	}

	void NounDatabase::Load() {
		std::cout << "Loading nouns...";
		mLoaded = true;

		const auto& dataPath = "data/noun/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					auto noun = std::make_shared<Noun>();
					noun->Read(document.child("noun"));

					auto nounName = path.stem().string();
					noun->mId = utils::hash_id(nounName);
					noun->mName = nounName;

					mNouns.try_emplace(noun->mId, noun);
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
				}
			}
		}

		std::cout << " done." << std::endl;;
	}

	NounPtr NounDatabase::Get(uint32_t noun) const {
		auto it = mNouns.find(noun);
		if (it != mNouns.end()) {
			return it->second;
		}
		return nullptr;
	}
}
