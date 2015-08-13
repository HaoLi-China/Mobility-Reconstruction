

template< class Real >
template< class PointReal >
int Octree< Real >::SetTree(const std::list<PointReal>& input_points, const std::list<PointReal>& input_normals, 
									 int minDepth , int maxDepth , int fullDepth , 
									 int splatDepth , Real samplesPerNode , Real scaleFactor ,
									 bool useConfidence , bool useNormalWeights , Real constraintWeight , int adaptiveExponent ,
									 PointInfo& pointInfo , NormalInfo& normalInfo , std::vector< Real >& kernelDensityWeights , std::vector< Real >& centerWeights ,
									 int boundaryType , XForm4x4< Real > xForm , bool makeComplete )
{
	if (input_points.size() != input_normals.size()) {
		std::cout << "points.size() != normals.size()" << std::endl;
		return 0;
	}

	if( splatDepth<0 ) splatDepth = 0;

	_boundaryType = boundaryType;
	if     ( _boundaryType<0 ) _boundaryType = -1;
	else if( _boundaryType>0 ) _boundaryType =  1;
	_samplesPerNode = samplesPerNode;
	_splatDepth = splatDepth;
	_constrainValues = (constraintWeight>0);

	XForm3x3< Real > xFormN;
	for( int i=0 ; i<3 ; i++ ) for( int j=0 ; j<3 ; j++ ) xFormN(i,j) = xForm(i,j);
	xFormN = xFormN.transpose().inverse();
	minDepth = std::min< int >( minDepth , maxDepth );	// minDepth <= maxDepth
	fullDepth = std::max< int >( minDepth , std::min< int >( fullDepth , maxDepth ) );	// minDepth <= fullDepth <= maxDepth
	// If _boundaryType==0, points are scaled to be in the [0.25,0.75]^3 cube so all depths have to be offset by
	// and the minDepth has to be 2.
	if( _boundaryType==0 )
	{
		minDepth++ , maxDepth++ , fullDepth++;
		if( splatDepth ) splatDepth++;
		minDepth = std::max< int >( minDepth , 2 );
	}
	// Otherwise the points are in the [0,1]^3 cube.
	// However, for Neumann constraints, the function at depth 0 is constant so the system matrix is zero if there
	// is no screening.
#if 0
	else if( _boundaryType==1 && !_constrainValues ) minDepth = std::max< int >( minDepth , 1 );
#endif

	_fData.set( maxDepth , _boundaryType );

	_minDepth = minDepth;
	_fullDepth = fullDepth;
	double pointWeightSum = 0;
	Point3D< Real > min , max , myCenter;
	Real myWidth;
	int i , cnt=0;
	TreeOctNode* temp;

	typename TreeOctNode::NeighborKey3 neighborKey;
	neighborKey.set( maxDepth );
	tree.setFullDepth( _fullDepth );

	// Read through once to get the center and scale
	{
		double t = Time();
		Point3D< Real > p , n;
        typename std::list< Point3D<Real> >::const_iterator pit=input_points.begin();
        typename std::list< Point3D<Real> >::const_iterator nit=input_normals.begin();
		for (; pit!=input_points.end(); ++pit, ++nit) {
			p = *pit;
			n = *nit;

			p = xForm * p;
			for( i=0 ; i<DIMENSION ; i++ )
			{
				if( !cnt || p[i]<min[i] ) min[i] = p[i];
				if( !cnt || p[i]>max[i] ) max[i] = p[i];
			}
			cnt++;
		}

		if( _boundaryType==0 )
			_scale = std::max< Real >( max[0]-min[0] , std::max< Real >( max[1]-min[1] , max[2]-min[2] ) ) * 2;
		else        
			_scale = std::max< Real >( max[0]-min[0] , std::max< Real >( max[1]-min[1] , max[2]-min[2] ) );
		_center = ( max+min ) /2;
	}

	_scale *= scaleFactor;
	for( i=0 ; i<DIMENSION ; i++ ) _center[i] -= _scale/2;
	if( splatDepth>0 )
	{
		double t = Time();
		cnt = 0;
		Point3D< Real > p , n;
        typename std::list< Point3D<Real> >::const_iterator pit=input_points.begin();
        typename std::list< Point3D<Real> >::const_iterator nit=input_normals.begin();
		for (; pit!=input_points.end(); ++pit, ++nit) {
			p = *pit;
			n = *nit;

			p = xForm * p , n = xFormN * n;
			p = ( p - _center ) / _scale;
			if( !_InBounds(p) ) continue;
			myCenter = Point3D< Real >( Real(0.5) , Real(0.5) , Real(0.5) );
			myWidth = Real(1.0);
			Real weight=Real( 1. );
			if( useConfidence ) weight = Real( Length(n) );
			temp = &tree;
			int d=0;
			while( d<splatDepth )
			{
				UpdateWeightContribution(kernelDensityWeights, temp , p , neighborKey , weight );
				if( !temp->children ) 
					temp->initChildren();
				int cIndex=TreeOctNode::CornerIndex( myCenter , p );
				temp = temp->children + cIndex;
				myWidth/=2;
				if( cIndex&1 ) myCenter[0] += myWidth/2;
				else           myCenter[0] -= myWidth/2;
				if( cIndex&2 ) myCenter[1] += myWidth/2;
				else           myCenter[1] -= myWidth/2;
				if( cIndex&4 ) myCenter[2] += myWidth/2;
				else           myCenter[2] -= myWidth/2;
				d++;
			}
			UpdateWeightContribution( kernelDensityWeights , temp , p , neighborKey , weight );
			cnt++;
		}
	}
	kernelDensityWeights.resize( TreeNodeData::NodeCount , 0 );

	std::vector< _PointData >& points = pointInfo.points;

	cnt = 0;
	Point3D< Real > p , n;
    typename std::list< Point3D<Real> >::const_iterator pit=input_points.begin();
    typename std::list< Point3D<Real> >::const_iterator nit=input_normals.begin();
	for (; pit!=input_points.end(); ++pit, ++nit) {
		p = *pit;
		n = *nit;

		n *= Real(-1.);
		p = xForm * p , n = xFormN * n;
		p = ( p - _center ) / _scale;
		if( !_InBounds(p) ) 
			continue;
		myCenter = Point3D< Real >( Real(0.5) , Real(0.5) , Real(0.5) );
		myWidth = Real(1.0);
		Real normalLength = Real( Length( n ) );
		if( normalLength!=normalLength || normalLength<=EPSILON ) 
			continue;
		if( !useConfidence ) n /= normalLength;

		Real pointWeight = Real(1.f);
		if( samplesPerNode>0 && splatDepth ) 
			pointWeight = SplatOrientedPoint( GetPointer( kernelDensityWeights ) , p , n , normalInfo , neighborKey , splatDepth , samplesPerNode , _minDepth , maxDepth );
		else
		{
			temp = &tree;
			int d=0;
			if( splatDepth )
			{
				while( d<splatDepth )
				{
					int cIndex=TreeOctNode::CornerIndex(myCenter,p);
					temp = &temp->children[cIndex];
					myWidth /= 2;
					if(cIndex&1) myCenter[0] += myWidth/2;
					else		 myCenter[0] -= myWidth/2;
					if(cIndex&2) myCenter[1] += myWidth/2;
					else		 myCenter[1] -= myWidth/2;
					if(cIndex&4) myCenter[2] += myWidth/2;
					else		 myCenter[2] -= myWidth/2;
					d++;
				}
				pointWeight = GetSampleWeight( GetPointer( kernelDensityWeights ) , temp , p , neighborKey );
			}
			for( i=0 ; i<DIMENSION ; i++ ) n[i] *= pointWeight;
			while( d<maxDepth )
			{
				if( !temp->children ) temp->initChildren();
				int cIndex=TreeOctNode::CornerIndex(myCenter,p);
				temp=&temp->children[cIndex];
				myWidth/=2;
				if(cIndex&1) myCenter[0] += myWidth/2;
				else		 myCenter[0] -= myWidth/2;
				if(cIndex&2) myCenter[1] += myWidth/2;
				else		 myCenter[1] -= myWidth/2;
				if(cIndex&4) myCenter[2] += myWidth/2;
				else		 myCenter[2] -= myWidth/2;
				d++;
			}
			SplatOrientedPoint( GetPointer( kernelDensityWeights ) , temp , p , n , normalInfo , neighborKey );
		}
		pointWeightSum += pointWeight;
		if( _constrainValues )
		{
			Real pointScreeningWeight = useNormalWeights ? Real( normalLength ) : Real(1.f);
			int d = 0;
			TreeOctNode* temp = &tree;
			myCenter = Point3D< Real >( Real(0.5) , Real(0.5) , Real(0.5) );
			myWidth = Real(1.0);
			while( 1 )
			{
				if( pointInfo.pointIndices.size()<TreeNodeData::NodeCount ) pointInfo.pointIndices.resize( TreeNodeData::NodeCount , -1 );
				int idx = pointInfo.pointIndex( temp );

				if( idx==-1 )
				{
					idx = (int)points.size();
					points.push_back( _PointData( p*pointScreeningWeight , pointScreeningWeight ) );
					pointInfo.pointIndices[ temp->nodeData.nodeIndex ] = idx;
				}
				else
				{
					points[idx].weight += pointScreeningWeight;
					points[idx].position += p*pointScreeningWeight;
				}

				int cIndex = TreeOctNode::CornerIndex( myCenter , p );
				if( !temp->children ) break;
				temp = &temp->children[cIndex];
				myWidth /= 2;
				if( cIndex&1 ) myCenter[0] += myWidth/2;
				else		   myCenter[0] -= myWidth/2;
				if( cIndex&2 ) myCenter[1] += myWidth/2;
				else		   myCenter[1] -= myWidth/2;
				if( cIndex&4 ) myCenter[2] += myWidth/2;
				else		   myCenter[2] -= myWidth/2;
				d++;
			}
		}
		cnt++;
	}

	if( _boundaryType==0 ) pointWeightSum *= Real(4.);
	constraintWeight *= Real( pointWeightSum );
	constraintWeight /= cnt;

	MemoryUsage( );
	if( _constrainValues )
		// Set the average position and scale the weights
		for( TreeOctNode* node=tree.nextNode() ; node ; node=tree.nextNode(node) )
			if( pointInfo.pointIndex( node )!=-1 )
			{
				int idx = pointInfo.pointIndex( node );
				points[idx].position /= points[idx].weight;
				int e = ( _boundaryType==0 ? node->depth()-1 : node->depth() ) * adaptiveExponent - ( _boundaryType==0 ? maxDepth-1 : maxDepth ) * (adaptiveExponent-1);
				if( e<0 ) points[idx].weight /= Real( 1<<(-e) );
				else      points[idx].weight *= Real( 1<<  e  );
				points[idx].weight *= Real( constraintWeight );
			}
#if FORCE_NEUMANN_FIELD
			if( _boundaryType==1 )
				for( TreeOctNode* node=tree.nextNode() ; node ; node=tree.nextNode( node ) )
				{
					int d , off[3] , res;
					node->depthAndOffset( d , off );
					res = 1<<d;
					int idx = normalInfo.normalIndex( node );
					if( idx<0 ) continue;
					Point3D< Real >& normal = normalInfo.normals[ idx ];
					for( int d=0 ; d<3 ; d++ ) if( off[d]==0 || off[d]==res-1 ) normal[d] = 0;
				}
#endif // FORCE_NEUMANN_FIELD
				centerWeights.resize( tree.nodes() , 0 );
				kernelDensityWeights.resize( tree.nodes() , 0 );
				// Set the point weights for evaluating the iso-value
				for( TreeOctNode* node=tree.nextNode() ; node ; node=tree.nextNode(node) )
				{
					int idx = normalInfo.normalIndex( node );
					if( idx<0 ) centerWeights[ node->nodeData.nodeIndex ] = 0;
					else        centerWeights[ node->nodeData.nodeIndex ] = Real( Length( normalInfo.normals[ idx ] ) );
				}
				MemoryUsage();
				{
					std::vector< int > indexMap;
					if( makeComplete ) MakeComplete( &indexMap );
					else ClipTree( normalInfo ) , Finalize( &indexMap );

					{
						std::vector< int > temp = pointInfo.pointIndices;
						pointInfo.pointIndices.resize( indexMap.size() );
						for( int i=0 ; i<indexMap.size() ; i++ )
							if( indexMap[i]<temp.size() ) pointInfo.pointIndices[i] = temp[ indexMap[i] ];
							else                          pointInfo.pointIndices[i] = -1;
					}
					{
						std::vector< int > temp = normalInfo.normalIndices;
						normalInfo.normalIndices.resize( indexMap.size() );
						for( int i=0 ; i<indexMap.size() ; i++ )
							if( indexMap[i]<temp.size() ) normalInfo.normalIndices[i] = temp[ indexMap[i] ];
							else                          normalInfo.normalIndices[i] = -1;
					}
					{
						std::vector< Real > temp = centerWeights;
						centerWeights.resize( indexMap.size() );
						for( int i=0 ; i<indexMap.size() ; i++ )
							if( indexMap[i]<temp.size() ) centerWeights[i] = temp[ indexMap[i] ];
							else                          centerWeights[i] = (Real)0;
					}
					{
						std::vector< Real > temp = kernelDensityWeights;
						kernelDensityWeights.resize( indexMap.size() );
						for( int i=0 ; i<indexMap.size() ; i++ )
							if( indexMap[i]<temp.size() ) kernelDensityWeights[i] = temp[ indexMap[i] ];
							else                          kernelDensityWeights[i] = (Real)0;
					}
				}
				return cnt;
}




