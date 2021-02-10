
// Include
#include "octree.h"
#include "objectmanager.h"

// TODO: just change this octree to focus on triggers

// Game
namespace Game {
	// OctTree
	OctTree::OctTree() : mRegion(glm::vec3(-2500), glm::vec3(2500)) {}
	OctTree::OctTree(const BoundingBox& boundingBox) : mRegion(boundingBox) {}

	void OctTree::Update() {
		if (!mReady) {
			if (!mBuilt) {
				while (!mQueue.empty()) {
					mObjects.push_back(mQueue.back());
					mQueue.pop_back();
				}

				BuildTree();
				mBuilt = true;
			} else {
				while (!mQueue.empty()) {
					Insert(mQueue.back());
					mQueue.pop_back();
				}
			}
			mReady = true;
		}

		if (mObjects.empty()) {
			if (mActiveNodes == 0) {
				if (mLifespan == -1) {
					mLifespan = mMaxLifespan;
				} else if (mLifespan > 0) {
					mLifespan--;
				}
			}
		} else {
			if (mLifespan != -1) {
				if (mMaxLifespan <= 64) {
					mMaxLifespan *= 2;
				}
				mLifespan = -1;
			}
		}

		decltype(mObjects) movedObjects;
		movedObjects.reserve(mObjects.size());

		for (const auto& object : mObjects) {
			if (object->IsDirty()) {
				object->SetDirty(false);
				movedObjects.push_back(object);
			}
		}

		for (int32_t index = static_cast<int32_t>(mObjects.size()) - 1; index >= 0; --index) {
			const auto& object = mObjects[index];
			if (object->IsMarkedForDeletion()) {
				auto it = std::find(movedObjects.begin(), movedObjects.end(), object);
				if (it != movedObjects.end()) {
					movedObjects.erase(it);
				}
				mObjects.erase(mObjects.begin() + index);
			}
		}

		for (uint32_t flags = mActiveNodes, index = 0; flags > 0; flags >>= 1, ++index) {
			auto& childNode = mChildNode[index];
			if ((flags & 1) && childNode->mLifespan == 0) {
				if (childNode->mObjects.empty()) {
					mActiveNodes ^= (1 << index);
					delete childNode;
					childNode = nullptr;
				} else {
					childNode->mLifespan = -1;
				}
			}
		}

		for (uint32_t flags = mActiveNodes, index = 0; flags > 0; flags >>= 1, ++index) {
			if (flags & 1) {
				mChildNode[index]->Update();
			}
		}

		for (const auto& movedObject : movedObjects) {
			auto current = this;

			const auto& bBox = movedObject->GetBoundingBox();
			if (bBox.mExtent != glm::zero<glm::vec3>()) {
				while (!current->mRegion.Contains(bBox)) {
					if (!current->mParentNode) {
						break;
					}
					current = current->mParentNode;
				}
			}
			/*
			const auto& position = movedObject->GetPosition();
			while (!current->mRegion.Contains(position)) {
				if (!current->mParentNode) {
					break;
				}
				current = current->mParentNode;
			}
			*/
			/*
			else {
				const auto& bSphere = movedObject->GetBoundingSphere();
				while (!current->mRegion.Contains(bSphere)) {
					if (!current->mParentNode) {
						break;
					}
					current = current->mParentNode;
				}
			}
			*/

			mObjects.erase(std::find(mObjects.begin(), mObjects.end(), movedObject));
			current->Insert(movedObject);
		}

		if (!mParentNode) {
			std::vector<std::tuple<ObjectPtr, ObjectPtr>> collisions;
			GetTriggerInteractions(collisions, {});

			for (const auto& [trigger, object] : collisions) {
				auto triggerObject = std::static_pointer_cast<TriggerVolume>(trigger);
				triggerObject->AddObject(object);
			}
		}
	}

	void OctTree::BuildTree() {
		if (mObjects.size() <= 1) {
			if (mObjects.size() == 1 && mRegion.Contains(mObjects.front()->GetBoundingBox())) {
				return;
			}
		}

		glm::vec3 extent = mRegion.mExtent;
		if (extent == glm::zero<glm::vec3>()) {
			// TODO: get and update extents.
		}

		if (extent.x <= sSmallestExtent && extent.y <= sSmallestExtent && extent.z <= sSmallestExtent) {
			return;
		}

		const glm::vec3& rMin = mRegion.GetMin();
		const glm::vec3& rMax = mRegion.GetMax();
		const glm::vec3& rCenter = rMin + extent;

		std::array<BoundingBox, 8> octant {
			BoundingBox(rMin, rCenter),																	// Bottom left (back)
			BoundingBox(glm::vec3(rCenter.x, rMin.y, rMin.z), glm::vec3(rMax.x, rCenter.y, rCenter.z)),	// Bottom right (back)
			BoundingBox(glm::vec3(rCenter.x, rMin.y, rCenter.z), glm::vec3(rMax.x, rCenter.y, rMax.z)),	// Bottom right (front)
			BoundingBox(glm::vec3(rMin.x, rMin.y, rCenter.z), glm::vec3(rCenter.x, rCenter.y, rMax.z)),	// Bottom left (front)
			BoundingBox(glm::vec3(rMin.x, rCenter.y, rMin.z), glm::vec3(rCenter.x, rMax.y, rCenter.z)),	// Top left (back)
			BoundingBox(glm::vec3(rCenter.x, rCenter.y, rMin.z), glm::vec3(rMax.x, rMax.y, rCenter.z)),	// Top right (back)
			BoundingBox(rCenter, rMax),																	// Top right (front)
			BoundingBox(glm::vec3(rMin.x, rCenter.y, rCenter.z), glm::vec3(rCenter.x, rMax.y, rMax.z))	// Top left (front)
		};

		std::array<decltype(mObjects), 8> objectList;
		std::vector<int32_t> objectRemoveList;

		for (int32_t index = 0, size = static_cast<int32_t>(mObjects.size()); index < size; ++index) {
			const auto& object = mObjects[index];
			const BoundingBox& boundingBox = object->GetBoundingBox();
			if (boundingBox.mExtent != glm::zero<glm::vec3>()) {
				for (uint32_t corner = 0; corner < 8; ++corner) {
					if (octant[corner].Contains(boundingBox)) {
						objectList[corner].push_back(object);
						objectRemoveList.push_back(index);
						break;
					}
				}
			}
			/*
			else if (obj.BoundingSphere.Radius != 0) {
				for (int a = 0; a < 8; a++) {
					if (octant[a].Contains(obj.BoundingSphere) == ContainmentType.Contains) {
						octList[a].Add(obj);
						delist.Add(obj);
						break;
					}
				}
			}
			*/
		}

		for (int32_t index = static_cast<int32_t>(objectRemoveList.size()) - 1; index >= 0; --index) {
			mObjects.erase(mObjects.begin() + objectRemoveList[index]);
		}

		//Create child nodes where there are items contained in the bounding region
		for (uint32_t corner = 0; corner < 8; ++corner) {
			const auto& objects = objectList[corner];
			if (objects.empty()) {
				continue;
			}

			mActiveNodes |= (1 << corner);
			mChildNode[corner] = CreateNode(octant[corner], objects);
			mChildNode[corner]->BuildTree();
		}
	}

	void OctTree::Enqueue(const ObjectPtr& object) {
		mQueue.push_back(object);
		mReady = false;
	}

	void OctTree::Insert(const ObjectPtr& object) {
		if (mObjects.size() <= 1 && mActiveNodes == 0) {
			mObjects.push_back(object);
			return;
		}

		const glm::vec3& extent = mRegion.mExtent;
		if (extent.x <= sSmallestExtent && extent.y <= sSmallestExtent && extent.z <= sSmallestExtent) {
			mObjects.push_back(object);
			return;
		}

		const BoundingBox& boundingBox = object->GetBoundingBox();

		bool isInRegion = mRegion.Contains(boundingBox);
		if (!isInRegion && mParentNode) {
			mParentNode->Insert(object);
			return;
		}

		const glm::vec3& rMin = mRegion.GetMin();
		const glm::vec3& rMax = mRegion.GetMax();
		const glm::vec3& rCenter = rMin + extent;

		//Find or create subdivided regions for each octant in the current region
		std::array<BoundingBox, 8> childOctant;
		childOctant[0] = mChildNode[0] ? mChildNode[0]->mRegion : BoundingBox(rMin, rCenter);																	// Bottom left (back)
		childOctant[1] = mChildNode[1] ? mChildNode[1]->mRegion : BoundingBox(glm::vec3(rCenter.x, rMin.y, rMin.z), glm::vec3(rMax.x, rCenter.y, rCenter.z));	// Bottom right (back)
		childOctant[2] = mChildNode[2] ? mChildNode[2]->mRegion : BoundingBox(glm::vec3(rCenter.x, rMin.y, rCenter.z), glm::vec3(rMax.x, rCenter.y, rMax.z));	// Bottom right (front)
		childOctant[3] = mChildNode[3] ? mChildNode[3]->mRegion : BoundingBox(glm::vec3(rMin.x, rMin.y, rCenter.z), glm::vec3(rCenter.x, rCenter.y, rMax.z));	// Bottom left (front)
		childOctant[4] = mChildNode[4] ? mChildNode[4]->mRegion : BoundingBox(glm::vec3(rMin.x, rCenter.y, rMin.z), glm::vec3(rCenter.x, rMax.y, rCenter.z));	// Top left (back)
		childOctant[5] = mChildNode[5] ? mChildNode[5]->mRegion : BoundingBox(glm::vec3(rCenter.x, rCenter.y, rMin.z), glm::vec3(rMax.x, rMax.y, rCenter.z));	// Top right (back)
		childOctant[6] = mChildNode[6] ? mChildNode[6]->mRegion : BoundingBox(rCenter, rMax);																	// Top right (front)
		childOctant[7] = mChildNode[7] ? mChildNode[7]->mRegion : BoundingBox(glm::vec3(rMin.x, rCenter.y, rCenter.z), glm::vec3(rCenter.x, rMax.y, rMax.z));	// Top left (front)

		if (isInRegion) {
			bool found = false;
			for (uint32_t corner = 0; corner < 8; ++corner) {
				const auto& childRegion = childOctant[corner];
				if (childRegion.Contains(boundingBox)) {
					auto& childNode = mChildNode[corner];
					if (childNode) {
						childNode->Insert(object);
					} else {
						childNode = CreateNode(childRegion, object);
						mActiveNodes |= (1 << corner);
					}
					found = true;
				}
			}

			if (!found) {
				mObjects.push_back(object);
			}
		}
		/*
		else if (Item.BoundingSphere.Radius != 0 && m_region.Contains(Item.BoundingSphere) == ContainmentType.Contains) {
			bool found = false;
			//we will try to place the object into a child node. If we can't fit it in a child node, then we insert it into the current node object list.
			for (int a = 0; a < 8; a++) {
				//is the object contained within a child quadrant?
				if (childOctant[a].Contains(Item.BoundingSphere) == ContainmentType.Contains) {
					if (m_childNode[a] != null)
						m_childNode[a].Insert(Item);   //Add the item into that tree and let the child tree figure out what to do with it
					else {
						m_childNode[a] = CreateNode(childOctant[a], Item);   //create a new tree node with the item
						m_activeNodes |= (byte)(1 << a);
					}
					found = true;
				}
			}
			if (!found) m_objects.Add(Item);
		}
		*/
		else {
			//either the item lies outside of the enclosed bounding box or it is intersecting it. Either way, we need to rebuild
			//the entire tree by enlarging the containing bounding box
			//BoundingBox enclosingArea = FindBox();
			BuildTree();
		}
	}

	std::vector<ObjectPtr> OctTree::GetObjectsInRadius(const glm::vec3& position, float radius, const std::vector<NounType>& types) const {
		std::vector<ObjectPtr> objects;
		
		BoundingBox region;
		region.mCenter = position;
		region.mExtent = glm::vec3(radius);

		GetObjectsInRadius(objects, region, types);
		return objects;
	}

	void OctTree::GetObjectsInRadius(std::vector<ObjectPtr>& objects, const BoundingBox& region, const std::vector<NounType>& types) const {
		if (!mBuilt) {
			return;
		}

		if (mObjects.empty() || mActiveNodes == 0) {
			return;
		}

		for (const auto& object : mObjects) {
			if (!region.Contains(object->GetBoundingBox())) {
				continue;
			}

			if (types.empty()) {
				objects.push_back(object);
			} else {
				auto type = object->GetType();
				for (auto acceptedType : types) {
					if (type == acceptedType) {
						objects.push_back(object);
						break;
					}
				}
			}
		}

		for (uint32_t flags = mActiveNodes, index = 0; flags > 0; flags >>= 1, ++index) {
			auto& childNode = mChildNode[index];
			if ((flags & 1) && childNode && region.Contains(childNode->mRegion)) {
				childNode->GetObjectsInRadius(objects, region, types);
			}
		}
	}

	void OctTree::GetTriggerInteractions(std::vector<std::tuple<ObjectPtr, ObjectPtr>>& collisions, std::vector<ObjectPtr> parentTriggers) const {
		// TODO: just store triggers in a seperate vector as to not fetch them each time?
		if (!mObjects.empty()) {
			std::remove_reference_t<decltype(parentTriggers)> localTriggers;
			if (parentTriggers.empty()) {
				for (const auto& object : mObjects) {
					if (object->IsTrigger()) {
						localTriggers.push_back(object);
					}
				}
			} else {
				for (const auto& trigger : parentTriggers) {
					const BoundingBox& triggerBox = trigger->GetBoundingBox();
					if (triggerBox.mExtent == glm::zero<glm::vec3>()) {
						continue;
					}

					const auto& triggerObject = std::static_pointer_cast<TriggerVolume>(trigger);
					for (const auto& object : mObjects) {
						if (object->IsTrigger()) {
							localTriggers.push_back(object);
							continue;
						}

						if (object != triggerObject->GetOwnerObject() && triggerBox.Contains(object->GetPosition())) {
							collisions.push_back(std::make_tuple(trigger, object));
						}
					}
				}
			}

			for (const auto& trigger : localTriggers) {
				const BoundingBox& triggerBox = trigger->GetBoundingBox();
				if (triggerBox.mExtent == glm::zero<glm::vec3>()) {
					continue;
				}

				const auto& triggerObject = std::static_pointer_cast<TriggerVolume>(trigger);
				for (const auto& object : mObjects) {
					if (object->IsTrigger()) {
						continue;
					}

					if (object != triggerObject->GetOwnerObject() && triggerBox.Contains(object->GetPosition())) {
						collisions.push_back(std::make_tuple(trigger, object));
					}
				}
			}

			parentTriggers.insert(parentTriggers.end(), localTriggers.begin(), localTriggers.end());
		}

		for (uint32_t flags = mActiveNodes, index = 0; flags > 0; flags >>= 1, ++index) {
			const auto& childNode = mChildNode[index];
			if ((flags & 1) && childNode) {
				childNode->GetTriggerInteractions(collisions, parentTriggers);
			}
		}
	}

	OctTree* OctTree::CreateNode(const BoundingBox& region, const std::vector<ObjectPtr>& objectList) {
		if (objectList.empty()) {
			return nullptr;
		}

		auto tree = new OctTree(region);
		tree->mParentNode = this;
		tree->mObjects = objectList;

		return tree;
	}

	OctTree* OctTree::CreateNode(const BoundingBox& region, const ObjectPtr& object) {
		if (!object) {
			return nullptr;
		}

		auto tree = new OctTree(region);
		tree->mParentNode = this;
		tree->mObjects.push_back(object);

		return tree;
	}
}
