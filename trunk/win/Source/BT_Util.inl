// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

template <class T>
vector<T> mergeVectors(const vector<T>& prev, const vector<T>& last)
{
	// merge the vectors 
	vector<T> newVec = prev;
	newVec.insert(newVec.end(), last.begin(), last.end());
	return newVec;
}

template <class T>
int vecContains(const vector<T>& vec, const T& value)
{
	for(int i=0; i < (int)vec.size(); i++)
	{
		if(vec[i]==value) return i;
	}
	return -1;
}

template <class T> 
T lerp(const T& begin, const T& end, const float t)
{
	return begin + t*(end-begin);
}

template <class T>
bool inRange(T value, T low, T high)
{
	return value >= low && value <= high;
}

template <class T>
T clampVals(T value, T low, T high)
{
	if(value <= low)
		return low;
	else if(value >= high)
		return high;
	else return value;
}

template <class T>
deque<T> lerpRange(const T& begin, const T& end, const int count, EaseFunctionType easeType)
{
	assert(count > 0);
	deque<T> range;
	if (count == 1)
	{	
		range.push_back(begin);
		return range;
	}

	for (int i = 0; i < count; i++)
	{
		float t = float(i) / float(count - 1);
		
		// Apply Easing
		switch (easeType)
		{
		case Ease:
			t = ease(t);
			break;
		case SoftEase:
			t = softEase(t);
			break;
		default:
			break;
		}

		range.push_back(begin + t*(end-begin));
	}

	return range;
}

template<class T>
deque<T> bounceGrow( T initialSize, T endSize, uint numSteps )
{
	assert(numSteps > 0);
	T vecSz = endSize - initialSize;
	float max = 10;
	float stepInc = max / numSteps;
	float val;
	deque<T> result;

	result.push_back(initialSize);

	// Diminishing Sine Curve
	for (float i = stepInc; i < max; i += stepInc)
	{
		val = (1.0f - sin(float(i)) / float(i));
		result.push_back(initialSize + (vecSz * val));
	}

	result.push_back(endSize);
	return result;
}

template<class T>
deque<T> bounce( T cur, T magnitude, uint numSteps )
{
	assert(numSteps > 0);
	deque<T> result;
	float max = 10;
	float stepInc = max / numSteps;
	float val;

	result.push_back(cur);
	result.push_back(cur + (magnitude * 0.3f));

	for (float i = stepInc; i < max; i += stepInc)
	{
		if (sqrt(i) == 0)
		{
			val = 0;
		}else{
			val = sin(i) / sqrt(i);
		}

		result.push_back(cur + (val * magnitude));
	}

	result.push_back(cur);
	return result;
}